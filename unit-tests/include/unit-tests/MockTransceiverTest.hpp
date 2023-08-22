#ifndef RUDPLIB_MOCKTRANSCEIVERTEST_HPP
#define RUDPLIB_MOCKTRANSCEIVERTEST_HPP

#include <gtest/gtest.h>

#include "rudplib/NetworkEndpoint.hpp"

class MockTransceiverTest : public ::testing::Test {
protected:
	rudp::NetworkEndpoint local_ipv4_endpoint;
	rudp::NetworkEndpoint remote_ipv4_endpoint;
	std::vector<uint8_t> test_data;
public:
	MockTransceiverTest();
};

#endif //RUDPLIB_MOCKTRANSCEIVERTEST_HPP
