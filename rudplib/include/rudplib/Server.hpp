#ifndef RUDPLIB_SERVER_HPP
#define RUDPLIB_SERVER_HPP

#include "Client.hpp"

namespace rudp {
	class Server {
	private:

	public:
		//TODO: add getter for local endpoint
		[[nodiscard]] Client Accept();

		[[nodiscard]] int Pending();

		void Start();

		void Stop();
	};
}

#endif //RUDPLIB_SERVER_HPP
