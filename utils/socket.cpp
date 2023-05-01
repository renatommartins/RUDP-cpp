#include <winsock2.h>

#include "socket.hpp"

bool socket_isSocketPendingBytes(int socket, int timeoutSeconds)
{
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(socket, &rfd);

    struct timeval timeout =
            {
                    .tv_sec = timeoutSeconds,
                    .tv_usec = 0,
            };

    int ret = select(socket + 1, &rfd, NULL, NULL, &timeout);

    return FD_ISSET(socket, &rfd)? true : false;
}
