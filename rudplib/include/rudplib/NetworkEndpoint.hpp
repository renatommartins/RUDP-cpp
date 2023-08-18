#ifndef RUDPLIB_NETWORKENDPOINT_HPP
#define RUDPLIB_NETWORKENDPOINT_HPP

#include <cstdint>
namespace rudp {
	enum class AddressFamily {
		IPv4,
		IPv6,
	};

	struct NetworkEndpoint {
		AddressFamily address_family;
		uint16_t port;
		union {
			uint8_t ip_v4_bytes[4]{};
			uint8_t ip_v6_bytes[16];
		};

		NetworkEndpoint();
		NetworkEndpoint(AddressFamily address_family, uint16_t port, uint8_t *ip_bytes);
	};
}

#endif //RUDPLIB_NETWORKENDPOINT_HPP
