#ifndef RUDPLIB_WINSOCK2SUPPORT_HPP
#define RUDPLIB_WINSOCK2SUPPORT_HPP

namespace rudp {
	class WinSock2Support {
	private:
		static int socket_count;
		WinSock2Support() = default;
	public:
		static void Initialize();
		static void Cleanup();
	};
}

#endif //RUDPLIB_WINSOCK2SUPPORT_HPP
