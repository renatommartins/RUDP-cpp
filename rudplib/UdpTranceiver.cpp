#include <vector>

#if(WIN32)
#include "WinSock2Support.hpp"
#endif

#include "utils/socket.hpp"

#include "UdpTransceiver.hpp"

namespace rudp {
	int UdpTransceiver::GetAvailable() const {
		if (rudp::utils::socket::isSocketPendingBytes(udp_socket, 0)) { return 1; }
		else { return 0; }
	}

	OpenResult UdpTransceiver::Open(const sockaddr &local, const sockaddr &remote) {
#if(WIN32)
		WinSock2Support::Initialize();
#endif

		udp_socket = socket(local.sa_family, SOCK_DGRAM, IPPROTO_UDP);
		if(udp_socket == INVALID_SOCKET)
			return  OpenResult::ResourceNotAvailable;

		auto bind_result = bind(udp_socket, &local, sizeof(sockaddr));
		if(bind_result == SOCKET_ERROR)
			return OpenResult::ResourceNotAvailable;

		this->local_endpoint = local;
		this->remote_endpoint = remote;

		return OpenResult::Successful;
	}

	std::vector<uint8_t> UdpTransceiver::Receive() {
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
			if(receive_address.sa_family != remote_endpoint.sa_family)
				break;

			auto compare_result = memcmp(
					remote_endpoint.sa_data,
					receive_address.sa_data,
					remote_endpoint.sa_family == AF_INET ?
					rudp::utils::socket::kIPV4Size :
					rudp::utils::socket::kIPV6Size);

			if(compare_result != 0)
				break;
		} while (false);

		auto receive_data = std::vector<uint8_t>{receive_buffer.data(), receive_buffer.data() + receive_count};

		receive_buffer.fill(0x00);

		return receive_data;
	}

	int UdpTransceiver::Transmit(std::span<uint8_t> data) {
		std::copy_n(data.data(), data.size(), send_buffer.data());

		auto send_buffer_pointer = reinterpret_cast<char*>(send_buffer.data());
		sendto(
			udp_socket,
			send_buffer_pointer,
			static_cast<int>(data.size()),
			0,
			&remote_endpoint,
			sizeof(remote_endpoint));

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