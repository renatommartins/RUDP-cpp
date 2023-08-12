#ifndef RUDPLIB_PACKETTYPE_HPP
#define RUDPLIB_PACKETTYPE_HPP

#include <cstdint>
namespace rudp {
	enum class PacketType : uint16_t {
		ConnectionRequest,
		ConnectionAccept,
		ConnectionRefuse,
		DisconnectionNotify,
		KeepAlive,
		Data,
		Invalid,
	};
}
#endif //RUDPLIB_PACKETTYPE_HPP
