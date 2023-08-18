#include <gtest/gtest.h>

#if WIN32 || WIN64
#include "WinSock2.h"
#endif

#include "rudplib/Client.hpp"
#include "rudplib/Packet.hpp"
#include "rudplib/MockTransceiver.hpp"

TEST(Client, Test_01) {
	std::shared_ptr<rudp::NetworkTransceiver> mock_transceiver = std::make_shared<rudp::MockTransceiver>();
	auto client = rudp::Client{ 0xAA55, mock_transceiver};

	/*auto local_endpoint = sockaddr_in{0};
	local_endpoint.sin_family = AF_INET;
	local_endpoint.sin_port = 1337;
	local_endpoint.sin_addr.S_un.S_addr = 0;

	client.Start(reinterpret_cast<const sockaddr&>(local_endpoint), reinterpret_cast<const sockaddr&>(local_endpoint));*/
}