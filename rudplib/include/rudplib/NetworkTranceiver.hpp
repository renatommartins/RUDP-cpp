#ifndef RUDPLIB_NETWORKTRANCEIVER_HPP
#define RUDPLIB_NETWORKTRANCEIVER_HPP

#include <span>
#include <string>
#include <vector>

#if(WIN32)
#include "WinSock2.h"
#endif

namespace rudp {
	enum class OpenResult {
		Successful,
		ResourceNotAvailable,
	};
	enum class CloseResult {
		Successful,
		TransceiverNotOpen,
		ResourceNotAvailable,
	};
	struct NetworkTransceiver {
		[[nodiscard]] virtual int GetAvailable() const = 0;
		virtual OpenResult Open(const sockaddr &local, const sockaddr &remote) = 0;
		[[nodiscard]] virtual std::vector<uint8_t> Receive() = 0;
		virtual int Transmit(std::span<uint8_t> data) = 0;
		virtual CloseResult Close() = 0;
	};
}

#endif //RUDPLIB_NETWORKTRANCEIVER_HPP
