#ifndef RUDPLIB_UDPTRANSCEIVERTEST_HPP
#define RUDPLIB_UDPTRANSCEIVERTEST_HPP

#include <gtest/gtest.h>

#include "rudplib/NetworkEndpoint.hpp"

class UdpTransceiverTest : public ::testing::Test {
protected:
	rudp::NetworkEndpoint local_ipv4_endpoint;
	rudp::NetworkEndpoint remote_ipv4_endpoint;
	std::vector<uint8_t> test_data;
public:
	UdpTransceiverTest();
};

#endif //RUDPLIB_UDPTRANSCEIVERTEST_HPP
