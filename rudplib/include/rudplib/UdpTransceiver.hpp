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
		static constexpr size_t kIPV4Size = 8;
		static constexpr size_t kIPV6Size = 28; //TODO: double-check this size
		static constexpr size_t kReceiveBufferSize = 1024 * 4;
		static constexpr size_t kSendBufferSize = 1024 * 4;
		static size_t ConvertNetworkEndpointToSockaddrBuffer(const NetworkEndpoint &network_endpoint, std::span<uint8_t> buffer);

		SOCKET udp_socket;
		bool is_open;
		std::array<uint8_t,64> local_endpoint_buffer;
		size_t local_endpoint_size;
		std::array<uint8_t,64> remote_endpoint_buffer;
		size_t remote_endpoint_size;
		std::array<uint8_t, kReceiveBufferSize> receive_buffer;
		std::array<uint8_t, kSendBufferSize> send_buffer;
	public:
		UdpTransceiver();
		~UdpTransceiver();
		[[nodiscard]] bool IsDataAvailable() const override;
		OpenResult Open(const NetworkEndpoint &local, const NetworkEndpoint &remote) override;
		[[nodiscard]] expected<vector<uint8_t>, ReceiveError> Receive() override;
		expected<int, TransmitError> Transmit(std::span<uint8_t> data) override;
		CloseResult Close() override;
	};
}

#endif //RUDPLIB_UDPTRANSCEIVER_HPP
