#include <array>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "rudplib/NetworkEndpoint.hpp"
#include "rudplib/UdpTransceiver.hpp"

#include "UdpTransceiverTest.hpp"

UdpTransceiverTest::UdpTransceiverTest() :
	test_data{ 't', 'e', 's', 't', ' ', 'd', 'a', 't', 'a' }
	{
	std::array<uint8_t, 4> local_address{ 127, 0, 0, 1 };
	std::array<uint8_t, 4> remote_address{ 127, 0, 0, 1 };

	local_ipv4_endpoint = rudp::NetworkEndpoint{
		rudp::AddressFamily::IPv4,
		1337,
		reinterpret_cast<uint8_t*>(&local_address)};

	remote_ipv4_endpoint = rudp::NetworkEndpoint{
		rudp::AddressFamily::IPv4,
		1338,
		reinterpret_cast<uint8_t*>(&remote_address)};
}

TEST_F (UdpTransceiverTest, Open_WhenClosed_ReturnSuccessful) {
	auto transceiver = rudp::UdpTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);

	ASSERT_EQ(open_result, rudp::OpenResult::Successful);
}

TEST_F (UdpTransceiverTest, Open_WhenAlreadyOpen_ReturnAlreadyOpenError) {
	auto transceiver = rudp::UdpTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);

	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::AlreadyOpen);
}

TEST_F (UdpTransceiverTest, Close_WhenOpen_ReturnSucessful) {
	auto transceiver = rudp::UdpTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);

	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto close_result = transceiver.Close();
	ASSERT_EQ(close_result, rudp::CloseResult::Successful);
}

TEST_F (UdpTransceiverTest, Close_WhenNotOpen_ReturnNotOpenError) {
	auto transceiver = rudp::UdpTransceiver{};

	auto close_result = transceiver.Close();
	ASSERT_EQ(close_result, rudp::CloseResult::TransceiverNotOpen);
}

TEST_F (UdpTransceiverTest, IsDataAvailable_WhenOpenAndNoDataIsAvailable_ReturnFalse) {
	auto transceiver = rudp::UdpTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto is_data_available_result = transceiver.IsDataAvailable();
	ASSERT_FALSE(is_data_available_result);
}

TEST_F (UdpTransceiverTest, IsDataAvailable_WhenOpenAndThereIsDataIsAvailable_ReturnTrue) {
	auto local_transceiver = rudp::UdpTransceiver{};
	auto open_result = local_transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto remote_transceiver = rudp::UdpTransceiver{};
	open_result = remote_transceiver.Open(remote_ipv4_endpoint, local_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto transmit_result = remote_transceiver.Transmit(test_data);
	ASSERT_TRUE(transmit_result.has_value());
	ASSERT_EQ(transmit_result.value(), test_data.size());

	auto is_data_available_result = local_transceiver.IsDataAvailable();
	ASSERT_TRUE(is_data_available_result);
}

TEST_F (UdpTransceiverTest, IsDataAvailable_WhenNotOpen_ReturnFalse) {
	auto transceiver = rudp::UdpTransceiver{};

	auto is_data_available_result = transceiver.IsDataAvailable();
	ASSERT_FALSE(is_data_available_result);
}

TEST_F (UdpTransceiverTest, Transmit_WhenOpen_ReturnDataLengthAndEnqueueData) {
	auto local_transceiver = rudp::UdpTransceiver{};
	auto open_result = local_transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto remote_transceiver = rudp::UdpTransceiver{};
	open_result = remote_transceiver.Open(remote_ipv4_endpoint, local_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto transmit_result = local_transceiver.Transmit(test_data);
	ASSERT_TRUE(transmit_result.has_value());
	ASSERT_EQ(transmit_result.value(), test_data.size());

	auto receive_result = remote_transceiver.Receive();
	ASSERT_TRUE(receive_result.has_value());
	ASSERT_THAT(receive_result.value(), testing::ContainerEq(test_data));
}

TEST_F (UdpTransceiverTest, Transmit_WhenNotOpen_ReturnNotOpenError) {
	auto transceiver = rudp::UdpTransceiver{};

	auto transmit_result = transceiver.Transmit(test_data);
	ASSERT_FALSE(transmit_result.has_value());
	ASSERT_EQ(transmit_result.error(), rudp::TransmitError::TransceiverNotOpen);
}

TEST_F (UdpTransceiverTest, Receive_WhenOpenAndReceivedData_ReturnReceivedData) {
	auto transceiver = rudp::UdpTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto remote_transceiver = rudp::UdpTransceiver{};
	open_result = remote_transceiver.Open(remote_ipv4_endpoint, local_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto transmit_result = remote_transceiver.Transmit(test_data);
	ASSERT_TRUE(transmit_result.has_value());
	ASSERT_EQ(transmit_result.value(), test_data.size());

	auto received_data = transceiver.Receive();
	ASSERT_TRUE(received_data.has_value());
	ASSERT_THAT(received_data.value(), testing::ContainerEq(test_data));
}

TEST_F (UdpTransceiverTest, Receive_WhenOpenAndDidntReceivedData_ReturnNoDataReceivedError) {
	auto transceiver = rudp::UdpTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto received_data = transceiver.Receive();
	ASSERT_FALSE(received_data.has_value());
	ASSERT_EQ(received_data.error(), rudp::ReceiveError::NoDataAvailable);
}

TEST_F (UdpTransceiverTest, Receive_WhenNotOpen_ReturnNotOpenError) {
	auto transceiver = rudp::UdpTransceiver{};

	auto received_data = transceiver.Receive();
	ASSERT_FALSE(received_data.has_value());
	ASSERT_EQ(received_data.error(), rudp::ReceiveError::TransceiverNotOpen);
}
