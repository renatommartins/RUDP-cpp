#ifndef RUDPLIB_MOCKTRANSCEIVER_HPP
#define RUDPLIB_MOCKTRANSCEIVER_HPP

#include <array>
#include <expected>
#include <queue>
#include <vector>

#include "NetworkEndpoint.hpp"
#include "NetworkTranceiver.hpp"

namespace rudp {
	class MockTransceiver : public NetworkTransceiver {
	private:
		bool is_open;

		NetworkEndpoint local_endpoint;
		NetworkEndpoint remote_endpoint;
	public:
		MockTransceiver();

		std::queue<std::vector<uint8_t>> receive_queue;
		std::queue<std::vector<uint8_t>> send_queue;
		[[nodiscard]] bool IsDataAvailable() const override;
		OpenResult Open(const NetworkEndpoint &local, const NetworkEndpoint &remote) override;
		[[nodiscard]] std::expected<std::vector<uint8_t>, ReceiveError> Receive() override;
		std::expected<int, TransmitError> Transmit(std::span<uint8_t> data) override;
		CloseResult Close() override;
	};
}

#endif //RUDPLIB_MOCKTRANSCEIVER_HPP
