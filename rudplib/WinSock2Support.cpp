#include <WinSock2.h>

#include "WinSock2Support.hpp"

namespace rudp {
	void WinSock2Support::Initialize() {
		if (is_initialized)
			return;

		auto wsa_data = WSADATA{0};
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
		is_initialized = true;
	}
}
