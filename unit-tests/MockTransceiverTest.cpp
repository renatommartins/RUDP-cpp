#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "rudplib/MockTransceiver.hpp"
#include "rudplib/NetworkEndpoint.hpp"

#include "MockTransceiverTest.hpp"

MockTransceiverTest::MockTransceiverTest() :
	test_data{ 't', 'e', 's', 't', ' ', 'd', 'a', 't', 'a' }
	{
	std::array<uint8_t, 4> local_address{ 0, 0, 0, 0};
	std::array<uint8_t, 4> remote_address{ 127, 0, 0, 1 };

	local_ipv4_endpoint = rudp::NetworkEndpoint{
		rudp::AddressFamily::IPv4,
        0,
        reinterpret_cast<uint8_t*>(&local_address)};

	remote_ipv4_endpoint = rudp::NetworkEndpoint{
		rudp::AddressFamily::IPv4,
		1337,
		reinterpret_cast<uint8_t*>(&remote_address)};
}

TEST_F (MockTransceiverTest, Open_WhenClosed_ReturnSuccessful) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);

	ASSERT_EQ(open_result, rudp::OpenResult::Successful);
}

TEST_F (MockTransceiverTest, Open_WhenAlreadyOpen_ReturnAlreadyOpenError) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);

	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::AlreadyOpen);
}

TEST_F (MockTransceiverTest, Close_WhenOpen_ReturnSucessful) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);

	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto close_result = transceiver.Close();
	ASSERT_EQ(close_result, rudp::CloseResult::Successful);
}

TEST_F (MockTransceiverTest, Close_WhenNotOpen_ReturnNotOpenError) {
	auto transceiver = rudp::MockTransceiver{};

	auto close_result = transceiver.Close();
	ASSERT_EQ(close_result, rudp::CloseResult::TransceiverNotOpen);
}

TEST_F (MockTransceiverTest, IsDataAvailable_WhenOpenAndNoDataIsAvailable_ReturnFalse) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto is_data_available_result = transceiver.IsDataAvailable();
	ASSERT_FALSE(is_data_available_result);
}

TEST_F (MockTransceiverTest, IsDataAvailable_WhenOpenAndThereIsDataIsAvailable_ReturnTrue) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	transceiver.receive_queue.emplace(test_data);

	auto is_data_available_result = transceiver.IsDataAvailable();
	ASSERT_TRUE(is_data_available_result);
}

TEST_F (MockTransceiverTest, IsDataAvailable_WhenNotOpen_ReturnFalse) {
	auto transceiver = rudp::MockTransceiver{};

	auto is_data_available_result = transceiver.IsDataAvailable();
	ASSERT_FALSE(is_data_available_result);
}

TEST_F (MockTransceiverTest, Transmit_WhenOpen_ReturnDataLengthAndEnqueueData) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto transmit_result = transceiver.Transmit(test_data);

	ASSERT_TRUE(transmit_result.has_value());
	ASSERT_EQ(transmit_result.value(), test_data.size());

	ASSERT_THAT(transceiver.send_queue.front(), testing::ContainerEq(test_data));
}

TEST_F (MockTransceiverTest, Transmit_WhenNotOpen_ReturnNotOpenError) {
	auto transceiver = rudp::MockTransceiver{};

	auto transmit_result = transceiver.Transmit(test_data);
	ASSERT_FALSE(transmit_result.has_value());
	ASSERT_EQ(transmit_result.error(), rudp::TransmitError::TransceiverNotOpen);
}

TEST_F (MockTransceiverTest, Receive_WhenOpenAndReceivedData_ReturnReceivedData) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	transceiver.receive_queue.emplace(test_data);

	auto received_data = transceiver.Receive();
	ASSERT_TRUE(received_data.has_value());
	ASSERT_THAT(received_data.value(), testing::ContainerEq(test_data));
}

TEST_F (MockTransceiverTest, Receive_WhenOpenAndDidntReceivedData_ReturnNoDataReceivedError) {
	auto transceiver = rudp::MockTransceiver{};
	auto open_result = transceiver.Open(local_ipv4_endpoint, remote_ipv4_endpoint);
	ASSERT_EQ(open_result, rudp::OpenResult::Successful);

	auto received_data = transceiver.Receive();
	ASSERT_FALSE(received_data.has_value());
	ASSERT_EQ(received_data.error(), rudp::ReceiveError::NoDataAvailable);
}

TEST_F (MockTransceiverTest, Receive_WhenNotOpen_ReturnNotOpenError) {
	auto transceiver = rudp::MockTransceiver{};

	auto received_data = transceiver.Receive();
	ASSERT_FALSE(received_data.has_value());
	ASSERT_EQ(received_data.error(), rudp::ReceiveError::TransceiverNotOpen);
}
