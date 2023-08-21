#ifndef RUDPLIB_UDPTRANSCEIVER_HPP
#define RUDPLIB_UDPTRANSCEIVER_HPP

#include <array>
#include <expected>
#include <vector>

#if WIN32 || WIN64
#include "WinSock2.h"
#endif

#include "NetworkEndpoint.hpp"
#include "NetworkTranceiver.hpp"

using std::expected;
using std::vector;

namespace rudp{
	class UdpTransceiver : NetworkTransceiver {
	private:
		SOCKET udp_socket;
		static constexpr size_t kReceiveBufferSize = 1024 * 4;
		static constexpr size_t kSendBufferSize = 1024 * 4;
		std::array<uint8_t, kReceiveBufferSize> receive_buffer;
		std::array<uint8_t, kSendBufferSize> send_buffer;
		bool is_open;
		NetworkEndpoint local_endpoint;
		uint8_t local_endpoint_buffer[64];
		size_t local_endpoint_size;
		NetworkEndpoint remote_endpoint;
		uint8_t remote_endpoint_buffer[64];
		size_t remote_endpoint_size;
		static size_t ConvertNetworkEndpointToSockaddrBuffer(const NetworkEndpoint &network_endpoint, uint8_t* buffer);
	public:
		UdpTransceiver();
		~UdpTransceiver();
		[[nodiscard]] int GetAvailable() const override;
		OpenResult Open(const NetworkEndpoint &local, const NetworkEndpoint &remote) override;
		[[nodiscard]] expected<vector<uint8_t>, ReceiveError> Receive() override;
		expected<int, TransmitError> Transmit(std::span<uint8_t> data) override;
		CloseResult Close() override;
	};
}

#endif //RUDPLIB_UDPTRANSCEIVER_HPP
