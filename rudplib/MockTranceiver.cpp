#include <expected>
#include <vector>

#include "MockTransceiver.hpp"

namespace rudp {
	MockTransceiver::MockTransceiver() :
		is_open{false},
		receive_queue{},
		send_queue{},
		local_endpoint{},
		remote_endpoint{}
	{}

	bool MockTransceiver::IsDataAvailable() const {
		return !receive_queue.empty();
	}

	OpenResult MockTransceiver::Open(const NetworkEndpoint &local, const NetworkEndpoint &remote) {
		if (is_open)
			return OpenResult::AlreadyOpen;

		is_open = true;

		return OpenResult::Successful;
	}

	std::expected<std::vector<uint8_t>, ReceiveError> MockTransceiver::Receive() {
		if (!is_open)
			return std::unexpected(ReceiveError::TransceiverNotOpen);

		if (receive_queue.empty())
			return std::unexpected(ReceiveError::NoDataAvailable);

		auto return_value = receive_queue.front();
		receive_queue.pop();

		return return_value;
	}

	std::expected<int, TransmitError> MockTransceiver::Transmit(std::span<uint8_t> data) {
		if (!is_open)
			return std::unexpected(TransmitError::TransceiverNotOpen);

		std::vector<uint8_t> send_data{data.begin(), data.end()};
		send_queue.emplace(send_data);

		return static_cast<int>(data.size());
	}

	CloseResult MockTransceiver::Close() {
		if (!is_open)
			return CloseResult::TransceiverNotOpen;

		is_open = false;

		return CloseResult::Successful;
	}
}