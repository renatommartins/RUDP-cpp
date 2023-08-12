#ifndef RUDPLIB_CLIENT_HPP
#define RUDPLIB_CLIENT_HPP

#include <cstdint>
#include <array>
#include <chrono>
#include <mutex>
#include <queue>
#include <span>
#include <thread>
#include <vector>

#if(WIN32)
#include <WinSock2.h>
#endif

#include "utils/time.hpp"
#include "utils/socket.hpp"

#include "ClientState.hpp"
#include "PacketResult.hpp"
#include "NetworkTranceiver.hpp"
#include "Packet.hpp"

namespace rudp {
	class Client {
	private:
		std::jthread connection_thread;

		uint16_t application_id;
		std::unique_ptr<NetworkTransceiver> network_transceiver;
		ClientState state;
		std::queue<std::vector<uint8_t>> receive_queue;
		std::queue<std::vector<uint8_t>> send_queue;
		std::mutex rtt_mutex;
		std::array<int, 8> rtt_window;
		int current_rtt;
		sockaddr local_endpoint;
		sockaddr remote_endpoint;

		uint16_t next_sequence_number;

		uint16_t last_sequence_number_acknowledged;
		uint32_t local_acknowledges; //TODO: consider if tracking more than 32 packets is needed

		uint16_t last_remote_sequence_number;
		uint32_t remote_acknowledges;
	public:
		Client(uint16_t application_id);

		static void ConnectionUpdate(std::stop_token stop_token, Client* client);

		static std::unique_ptr<const Packet> ReceivePacket(Client* client);

		[[nodiscard]] inline int GetAvailable() const;

		[[nodiscard]] int GetRtt() const;

		[[nodiscard]] inline ClientState GetState() const;

		[[nodiscard]] inline sockaddr GetLocalEndpoint() const;

		[[nodiscard]] inline sockaddr GetRemoteEndpoint() const;

		//int Bind(const sockaddr &endpoint);

		int Start(const sockaddr &local, const sockaddr &remote); //TODO: define parameters

		void Close();

		uint16_t Send(std::span<uint8_t> data_to_send);

		[[nodiscard]] std::vector<uint8_t> Receive();

		[[nodiscard]] std::array<const PacketResult, sizeof(uint32_t) * 8> GetAcknowledge() const;
	};
}
#endif //RUDPLIB_CLIENT_HPP
