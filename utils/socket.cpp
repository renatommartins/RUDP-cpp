#if WIN32
#include <winsock2.h>
#else
#include <cstddef>
//#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#endif


#include "socket.hpp"

//TODO: review this implementation, "ret" variable is not used and it doesn't seem right
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
