#ifndef UTILS_SOCKET_HPP
#define UTILS_SOCKET_HPP

#if WIN32
#include <WinSock2.h>
#else
#include <cstddef>
#include <sys/time.h>
#include <sys/ioctl.h>
#endif

namespace rudp::utils::socket {
	constexpr size_t kIPV4Size = 4;
	constexpr size_t kIPV6Size = 16;
	bool isSocketPendingBytes(SOCKET socket, int timeoutSeconds);
}

#endif //UTILS_SOCKET_HPP
