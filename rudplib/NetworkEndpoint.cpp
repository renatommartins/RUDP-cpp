#include <cstring>
#include <exception>

#include "NetworkEndpoint.hpp"

namespace rudp {
	NetworkEndpoint::NetworkEndpoint() :
		address_family{},
		port{0}
	{
		ip_v4 = 0;
	}

	NetworkEndpoint::NetworkEndpoint(AddressFamily address_family, uint16_t port, uint8_t *ip_bytes) :
		address_family{address_family},
		port{port}
	{
		switch (address_family) {
			case AddressFamily::IPv4:
				memcpy(ip_v4_bytes.data(), ip_bytes, 4);
				break;
			case AddressFamily::IPv6:
				memcpy(ip_v6_bytes.data(), ip_bytes, 16);
				break;
			default:
				throw std::exception{"abc"};
		}
	}
}