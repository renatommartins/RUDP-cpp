#include <iostream>
#include <vector>

#if(WIN32 || WIN64)
#include "WinSock2.h"
#include "WS2tcpip.h"
#endif

#include "UdpTransceiver.hpp"

namespace rudp {
	size_t UdpTransceiver::ConvertNetworkEndpointToSockaddrBuffer(
		const NetworkEndpoint &network_endpoint,
		std::span<uint8_t> buffer) {
		auto local_endpoint_sockaddr = reinterpret_cast<sockaddr*>(buffer.data());

		switch (network_endpoint.address_family) {
			case AddressFamily::IPv4: {
				auto ipv4_endpoint = reinterpret_cast<sockaddr_in*>(local_endpoint_sockaddr);
				ipv4_endpoint->sin_family = AF_INET;
				ipv4_endpoint->sin_port = htons(network_endpoint.port);
				auto sockaddr_ip_v4 = reinterpret_cast<uint32_t*>(&ipv4_endpoint->sin_addr);
				*sockaddr_ip_v4 = network_endpoint.ip_v4;
				return sizeof(sockaddr_in);
			}
			case AddressFamily::IPv6: {
				auto ipv6_endpoint = reinterpret_cast<sockaddr_in6*>(local_endpoint_sockaddr);
				ipv6_endpoint->sin6_family = AF_INET6;
				ipv6_endpoint->sin6_port = network_endpoint.port;
				auto sockaddr_ip_v6_upper = reinterpret_cast<uint64_t*>(&ipv6_endpoint->sin6_addr);
				auto sockaddr_ip_v6_lower = reinterpret_cast<uint64_t*>(&ipv6_endpoint->sin6_addr);
				*sockaddr_ip_v6_upper = network_endpoint.ip_v6_upper;
				*sockaddr_ip_v6_lower = network_endpoint.ip_v6_lower;
				std::swap(*sockaddr_ip_v6_upper, *sockaddr_ip_v6_lower);//TODO: does this work?
				return sizeof(sockaddr_in6);
			}
			default:
				return 0;
		}
	}

	UdpTransceiver::UdpTransceiver() :
		udp_socket{INVALID_SOCKET},//TODO: check if preprocessor if(WIN32) is needed
		is_open{false},
		local_endpoint_buffer{0},
		local_endpoint_size{0},
		remote_endpoint_buffer{0},
		remote_endpoint_size{0},
		receive_buffer{0},
		send_buffer{0}
	{
#if WIN32
		auto wsa_data = WSADATA{0};
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
	}

	UdpTransceiver::~UdpTransceiver() {
		if(is_open) {
			auto close_result = closesocket(udp_socket);
			if(close_result == SOCKET_ERROR)
				std::cout << close_result;
		}
#if WIN32 || WIN64
		WSACleanup();
#endif
	}

	bool UdpTransceiver::IsDataAvailable() const {
		if(!is_open)
			return false;

		fd_set rfd;
		FD_ZERO(&rfd);
		FD_SET(udp_socket, &rfd);

		struct timeval timeout = {
			.tv_sec = 0,
			.tv_usec = 100,
		};

		int ret = select(static_cast<int>(udp_socket) + 1, &rfd, nullptr, nullptr, &timeout);
		auto is_set = FD_ISSET(udp_socket, &rfd);

		return is_set != 0;
	}

	OpenResult UdpTransceiver::Open(const NetworkEndpoint &local, const NetworkEndpoint &remote) {
		if(is_open)
			return OpenResult::AlreadyOpen;

		local_endpoint_size = ConvertNetworkEndpointToSockaddrBuffer(local, local_endpoint_buffer);
		remote_endpoint_size = ConvertNetworkEndpointToSockaddrBuffer(remote, remote_endpoint_buffer);

		auto local_sockaddr = reinterpret_cast<sockaddr*>(local_endpoint_buffer.data());

		udp_socket = socket(local_sockaddr->sa_family, SOCK_DGRAM, IPPROTO_UDP);
		if(udp_socket == INVALID_SOCKET)
			return  OpenResult::ResourceNotAvailable;

		auto bind_result = bind(udp_socket, local_sockaddr, static_cast<int>(local_endpoint_size));
		if(bind_result == SOCKET_ERROR)
			return OpenResult::ResourceNotAvailable;

		is_open = true;

		return OpenResult::Successful;
	}

	std::expected<std::vector<uint8_t>, ReceiveError> UdpTransceiver::Receive() {
		if(udp_socket == INVALID_SOCKET)
			return std::unexpected(ReceiveError::TransceiverNotOpen);

		if(IsDataAvailable() == 0)
			return std::unexpected(ReceiveError::NoDataAvailable);

		auto remote_sockaddr = reinterpret_cast<sockaddr*>(remote_endpoint_buffer.data());

		sockaddr receive_address{0};
		int receive_address_size{sizeof(receive_address)};
		auto receive_buffer_pointer = reinterpret_cast<char*>(receive_buffer.data());
		auto receive_count = recvfrom(
			udp_socket,
			receive_buffer_pointer,
			kReceiveBufferSize,
			0,
			&receive_address,
			&receive_address_size);

		if(receive_count == SOCKET_ERROR)
			return std::unexpected(ReceiveError::TransceiverError);

		if(receive_address.sa_family != remote_sockaddr->sa_family)
			return std::unexpected(ReceiveError::NoDataAvailable);

		auto compare_result = memcmp(
			remote_sockaddr,
			&receive_address,
			remote_sockaddr->sa_family == AF_INET ? kIPV4Size : kIPV6Size);

		if(compare_result != 0)
			return std::unexpected(ReceiveError::NoDataAvailable);

		auto receive_data = std::vector<uint8_t>{receive_buffer.data(), receive_buffer.data() + receive_count};

		receive_buffer.fill(0x00);

		return receive_data;
	}

	std::expected<int, TransmitError> UdpTransceiver::Transmit(std::span<uint8_t> data) {
		if(udp_socket == INVALID_SOCKET)
			return std::unexpected(TransmitError::TransceiverNotOpen);

		auto remote_sockaddr = reinterpret_cast<sockaddr*>(remote_endpoint_buffer.data());

		std::copy_n(data.data(), data.size(), send_buffer.data());

		auto send_buffer_pointer = reinterpret_cast<char*>(send_buffer.data());
		sendto(
			udp_socket,
			send_buffer_pointer,
			static_cast<int>(data.size()),
			0,
			remote_sockaddr,
			static_cast<int>(remote_endpoint_size));

		send_buffer.fill(0x00);

		return static_cast<int>(data.size());
	}

	CloseResult UdpTransceiver::Close() {
		if(!is_open)
			return CloseResult::TransceiverNotOpen;

		auto close_result = closesocket(udp_socket);

		if(close_result == SOCKET_ERROR)
			return CloseResult::ResourceNotAvailable;

		is_open = false;

		return CloseResult::Successful;
	}
}