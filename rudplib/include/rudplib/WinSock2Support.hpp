#ifndef RUDPLIB_WINSOCK2SUPPORT_HPP
#define RUDPLIB_WINSOCK2SUPPORT_HPP

namespace rudp {
	class WinSock2Support {
	private:
		static bool is_initialized;
	public:
		static void Initialize();
	};
}

#endif //RUDPLIB_WINSOCK2SUPPORT_HPP