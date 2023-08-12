#ifndef RUDPLIB_PACKETRESULT_HPP
#define RUDPLIB_PACKETRESULT_HPP

#include <cstdint>

namespace rudp {
	struct PacketResult {
		uint16_t sequence_number;
		bool is_acknowledged;
	};
}

#endif //RUDPLIB_PACKETRESULT_HPP
