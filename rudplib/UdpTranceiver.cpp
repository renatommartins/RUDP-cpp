#include <vector>

#if(WIN32 || WIN64)
#include "WinSock2Support.hpp"
#include "WinSock2.h"
#include "WS2tcpip.h"
#endif

#include "utils/socket.hpp"

#include "UdpTransceiver.hpp"

namespace rudp {
	size_t UdpTransceiver::ConvertNetworkEndpointToSockaddrBuffer(const NetworkEndpoint &network_endpoint, uint8_t* buffer){
		auto local_endpoint_sockaddr = reinterpret_cast<sockaddr*>(buffer);

		switch (network_endpoint.address_family) {
			case AddressFamily::IPv4: {
				auto ipv4_endpoint = reinterpret_cast<sockaddr_in*>(local_endpoint_sockaddr);
				ipv4_endpoint->sin_family = AF_INET;
				ipv4_endpoint->sin_port = network_endpoint.port;
				memcpy(&ipv4_endpoint->sin_addr, network_endpoint.ip_v4_bytes, 4);
				return sizeof(sockaddr_in);
			}
			case AddressFamily::IPv6: {
				auto ipv6_endpoint = reinterpret_cast<sockaddr_in6*>(local_endpoint_sockaddr);
				ipv6_endpoint->sin6_family = AF_INET6;
				ipv6_endpoint->sin6_port = network_endpoint.port;
				memcpy(&ipv6_endpoint->sin6_addr, network_endpoint.ip_v6_bytes, 16);
				return sizeof(sockaddr_in6);
			}
			default:
				return 0;
		}
	}

	UdpTransceiver::UdpTransceiver() :
		udp_socket{INVALID_SOCKET}//TODO: check if preprocessor if(WIN32) is needed
	{

	}

	int UdpTransceiver::GetAvailable() const {
		if (rudp::utils::socket::isSocketPendingBytes(udp_socket, 0)) { return 1; }
		else { return 0; }
	}

	OpenResult UdpTransceiver::Open(const NetworkEndpoint &local, const NetworkEndpoint &remote) {
#if WIN32 || WIN64
		WinSock2Support::Initialize();
#endif

		local_endpoint_size = ConvertNetworkEndpointToSockaddrBuffer(local, local_endpoint_buffer);
		remote_endpoint_size = ConvertNetworkEndpointToSockaddrBuffer(remote, remote_endpoint_buffer);

		auto local_sockaddr = reinterpret_cast<sockaddr*>(local_endpoint_buffer);

		udp_socket = socket(local_sockaddr->sa_family, SOCK_DGRAM, IPPROTO_UDP);
		if(udp_socket == INVALID_SOCKET)
			return  OpenResult::ResourceNotAvailable;

		auto bind_result = bind(udp_socket, local_sockaddr, static_cast<int>(local_endpoint_size));
		if(bind_result == SOCKET_ERROR)
			return OpenResult::ResourceNotAvailable;

		this->local_endpoint = local;
		this->remote_endpoint = remote;

		return OpenResult::Successful;
	}

	std::expected<std::vector<uint8_t>, ReceiveError> UdpTransceiver::Receive() {
		if(udp_socket == INVALID_SOCKET)
			return std::unexpected(ReceiveError::TransceiverNotOpen);

		if(GetAvailable() == 0)
			return std::unexpected(ReceiveError::NoDataAvailable);

		auto remote_sockaddr = reinterpret_cast<sockaddr*>(remote_endpoint_buffer);

		sockaddr receive_address{0};
		int receive_address_size{0};
		auto receive_buffer_pointer = reinterpret_cast<char*>(receive_buffer.data());
		auto receive_count = recvfrom(
			udp_socket,
			receive_buffer_pointer,
			kReceiveBufferSize,
			0,
			&receive_address,
			&receive_address_size);

		do {
			if(receive_address.sa_family != remote_sockaddr->sa_family)
				break;

			auto compare_result = memcmp(
				remote_sockaddr->sa_data,
				receive_address.sa_data,
				remote_sockaddr->sa_family == AF_INET ?
				rudp::utils::socket::kIPV4Size :
				rudp::utils::socket::kIPV6Size);

			if(compare_result != 0)
				return std::unexpected(ReceiveError::NoDataAvailable);
		} while (false);

		auto receive_data = std::vector<uint8_t>{receive_buffer.data(), receive_buffer.data() + receive_count};

		receive_buffer.fill(0x00);

		return receive_data;
	}

	std::expected<int, TransmitError> UdpTransceiver::Transmit(std::span<uint8_t> data) {
		if(udp_socket == INVALID_SOCKET)
			return std::unexpected(TransmitError::TransceiverNotOpen);

		auto remote_sockaddr = reinterpret_cast<sockaddr*>(remote_endpoint_buffer);

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
		auto close_result = closesocket(udp_socket);

		if(close_result == SOCKET_ERROR)
			return CloseResult::ResourceNotAvailable;

		return CloseResult::Successful;
	}
}