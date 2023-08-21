#ifndef RUDPLIB_NETWORKENDPOINT_HPP
#define RUDPLIB_NETWORKENDPOINT_HPP

#include <cstdint>
#include <array>

namespace rudp {
	enum class AddressFamily {
		IPv4,
		IPv6,
	};

	struct NetworkEndpoint {
		AddressFamily address_family;
		uint16_t port;
		union {
			std::array<uint8_t, 4> ip_v4_bytes;
			uint32_t ip_v4;
			std::array<uint8_t, 16> ip_v6_bytes;
			uint64_t ip_v6_upper;
			uint64_t ip_v6_lower;
		};

		NetworkEndpoint();
		NetworkEndpoint(AddressFamily address_family, uint16_t port, uint8_t *ip_bytes);
	};
}

#endif //RUDPLIB_NETWORKENDPOINT_HPP
