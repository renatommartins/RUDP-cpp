#include <WinSock2.h>

#include "WinSock2Support.hpp"

namespace rudp {
	int WinSock2Support::socket_count;

	void WinSock2Support::Initialize() {
		if (socket_count == 0) {
			auto wsa_data = WSADATA{0};
			WSAStartup(MAKEWORD(2, 2), &wsa_data);
		}
		socket_count++;
	}

	void WinSock2Support::Cleanup() {
		socket_count--;
		if (socket_count == 0) {
			WSACleanup();
		}
	}
}
