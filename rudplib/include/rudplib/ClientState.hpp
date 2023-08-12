#ifndef RUDPLIB_CLIENTSTATE_HPP
#define RUDPLIB_CLIENTSTATE_HPP

namespace rudp {
	enum class ClientState {
		Disconnected,
		Connecting,
		Connected,
		Disconnecting,
		ForceClose,
		Invalid,
	};
}

#endif //RUDPLIB_CLIENTSTATE_HPP
