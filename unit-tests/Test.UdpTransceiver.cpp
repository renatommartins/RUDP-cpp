#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "rudplib/NetworkEndpoint.hpp"
#include "rudplib/UdpTransceiver.hpp"

TEST (UdpTransceiver, ABC) {
	rudp::UdpTransceiver transceiver{};

	std::array<uint8_t, 4> local_ip{ 0, 0, 0, 0 };
	rudp::NetworkEndpoint local_endpoint{
		rudp::AddressFamily::IPv4,
		1337,
		reinterpret_cast<uint8_t*>(&local_ip)};

	std::array<uint8_t, 4> remote_ip{ 127, 0, 0, 1 };
	rudp::NetworkEndpoint remote_endpoint{
		rudp::AddressFamily::IPv4,
		1338,
		reinterpret_cast<uint8_t*>(&remote_ip)};

	transceiver.Open(local_endpoint, remote_endpoint);

	for (auto i{0}; i < 10; i++) {
		std::vector<uint8_t> test_transmit_data{ 'a', 'b', 'c' };
		transceiver.Transmit(test_transmit_data);
	}

	while(transceiver.GetAvailable() > 0) {
		auto received_data = transceiver.Receive();
	}
}