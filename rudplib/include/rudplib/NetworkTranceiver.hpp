#ifndef RUDPLIB_NETWORKTRANCEIVER_HPP
#define RUDPLIB_NETWORKTRANCEIVER_HPP

#include <expected>
#include <span>
#include <vector>

#include "NetworkEndpoint.hpp"

namespace rudp {
	enum class OpenResult {
		Successful,
		AlreadyOpen,
		ResourceNotAvailable,
		InvalidEndpoint,
	};
	enum class CloseResult {
		Successful,
		TransceiverNotOpen,
		ResourceNotAvailable,
	};
	enum class ReceiveError {
		TransceiverNotOpen,
		NoDataAvailable,
		TransceiverError,
	};
	enum class TransmitError {
		TransceiverNotOpen,
	};
	struct NetworkTransceiver {
		[[nodiscard]] virtual int GetAvailable() const = 0;
		virtual OpenResult Open(const NetworkEndpoint &local, const NetworkEndpoint &remote) = 0;
		[[nodiscard]] virtual std::expected<std::vector<uint8_t>, ReceiveError> Receive() = 0;
		virtual std::expected<int, TransmitError> Transmit(std::span<uint8_t> data) = 0;
		virtual CloseResult Close() = 0;
	};
}

#endif //RUDPLIB_NETWORKTRANCEIVER_HPP
