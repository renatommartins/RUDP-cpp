#ifndef RUDPLIB_UDPTRANSCEIVER_HPP
#define RUDPLIB_UDPTRANSCEIVER_HPP

#include <array>

#include "WinSock2.h"

#include "NetworkTranceiver.hpp"

namespace rudp{
	class UdpTransceiver : NetworkTransceiver {
	private:
		SOCKET udp_socket;
		static constexpr size_t kReceiveBufferSize = 1024 * 4;
		static constexpr size_t kSendBufferSize = 1024 * 4;
		std::array<uint8_t, kReceiveBufferSize> receive_buffer;
		std::array<uint8_t, kSendBufferSize> send_buffer;
		sockaddr local_endpoint;
		sockaddr remote_endpoint;
	public:
		[[nodiscard]] int GetAvailable() const override;
		OpenResult Open(const sockaddr &local, const sockaddr &remote) override;
		[[nodiscard]] std::vector<uint8_t> Receive() override;
		int Transmit(std::span<uint8_t> data) override;
		CloseResult Close() override;
	};
}

#endif //RUDPLIB_UDPTRANSCEIVER_HPP
