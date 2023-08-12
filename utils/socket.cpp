#include "socket.hpp"

//TODO: review this implementation, "ret" variable is not used and it doesn't seem right
bool rudp::utils::socket::isSocketPendingBytes(SOCKET socket, int timeoutSeconds) {
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(socket, &rfd);

	struct timeval timeout = {
		.tv_sec = timeoutSeconds,
		.tv_usec = 0,
	};

	select(socket + 1, &rfd, nullptr, nullptr, &timeout);

	return FD_ISSET(socket, &rfd)? true : false;
}
