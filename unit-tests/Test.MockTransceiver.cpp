#include <gtest/gtest.h>

#include "rudplib/MockTransceiver.hpp"
#include "rudplib/NetworkEndpoint.hpp"

TEST(MockTransceiver, ABC) {
	auto transceiver = rudp::MockTransceiver{};

	std::array<uint8_t, 4> local_address{ 0, 0, 0, 0};
	std::array<uint8_t, 4> remote_address{ 127, 0, 0, 1 };

	transceiver.Open(
		rudp::NetworkEndpoint{
			rudp::AddressFamily::IPv4,
			0,
			reinterpret_cast<uint8_t*>(&local_address)},
		rudp::NetworkEndpoint{
			rudp::AddressFamily::IPv4,
			1337,
			reinterpret_cast<uint8_t*>(&remote_address)});

	std::vector<uint8_t> receive_test_data{ 0xAA, 0x55, 0xAA, 0x55 };
	transceiver.receive_queue.emplace(receive_test_data);

	if (transceiver.IsDataAvailable() != 0) {
		auto receive_test_data_validation = transceiver.Receive();
	}

	std::vector<uint8_t> transmit_test_data{ 0x55, 0xAA, 0x55, 0xAA  };
	auto send_count = transceiver.Transmit(transmit_test_data);

	if (!transceiver.send_queue.empty()) {
		auto transmit_test_data_validation = transceiver.send_queue.front();
		transceiver.send_queue.pop();
	}

	transceiver.Close();
}

TEST (MockTransceiver, Open_WhenClosed_Successful) {
	auto transceiver = rudp::MockTransceiver{};

	std::array<uint8_t, 4> local_address{ 0, 0, 0, 0};
	std::array<uint8_t, 4> remote_address{ 127, 0, 0, 1 };

	auto open_result = transceiver.Open(
		rudp::NetworkEndpoint{
			rudp::AddressFamily::IPv4,
			0,
			reinterpret_cast<uint8_t*>(&local_address)},
		rudp::NetworkEndpoint{
			rudp::AddressFamily::IPv4,
			1337,
			reinterpret_cast<uint8_t*>(&remote_address)});

	ASSERT_EQ(open_result, rudp::OpenResult::Successful);
}

TEST (MockTransceiver, Open_WhenAlreadyOpen_AlreadyOpen) {
	auto transceiver = rudp::MockTransceiver{};

	std::array<uint8_t, 4> local_address{ 0, 0, 0, 0};
	std::array<uint8_t, 4> remote_address{ 127, 0, 0, 1 };

	rudp::NetworkEndpoint local_endpoint {
		rudp::AddressFamily::IPv4,
		0,
		reinterpret_cast<uint8_t*>(&local_address)};

	rudp::NetworkEndpoint remote_endpoint {
		rudp::AddressFamily::IPv4,
		1337,
		reinterpret_cast<uint8_t*>(&remote_address)};

	auto open_result = transceiver.Open(local_endpoint, remote_endpoint);
	EXPECT_EQ(open_result, rudp::OpenResult::Successful);

	open_result = transceiver.Open(local_endpoint, remote_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::AlreadyOpen);
}
