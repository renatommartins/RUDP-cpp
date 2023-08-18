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

#include "ClientState.hpp"
#include "PacketResult.hpp"
#include "NetworkEndpoint.hpp"
#include "NetworkTranceiver.hpp"
#include "Packet.hpp"

namespace rudp {
	class Client {
	private:
		std::jthread connection_thread;

		uint16_t application_id;
		std::shared_ptr<NetworkTransceiver> network_transceiver;
		ClientState state;
		std::queue<std::vector<uint8_t>> receive_queue;
		std::queue<std::vector<uint8_t>> send_queue;
		std::mutex rtt_mutex;
		std::array<int, 8> rtt_window;
		int current_rtt;
		NetworkEndpoint local_endpoint;
		NetworkEndpoint remote_endpoint;

		uint16_t next_sequence_number;

		uint16_t last_sequence_number_acknowledged;
		uint32_t local_acknowledges; //TODO: consider if tracking more than 32 packets is needed

		uint16_t last_remote_sequence_number;
		uint32_t remote_acknowledges;

		static void ConnectionUpdate(std::stop_token stop_token, Client* client);
		static void SendPacket(
				std::shared_ptr<NetworkTransceiver> &network_transceiver,
				uint16_t app_id,
				uint16_t sequence_number,
				uint16_t ack_sequence_number,
				uint32_t ack,
				PacketType type,
				std::optional<std::vector<uint8_t>> data);
	public:
		Client(
				uint16_t application_id,
				std::shared_ptr<NetworkTransceiver> network_transceiver);

		static std::unique_ptr<const Packet> ReceivePacket(Client* client);

		[[nodiscard]] inline int GetAvailable() const;

		[[nodiscard]] int GetRtt() const;

		[[nodiscard]] inline ClientState GetState() const;

		[[nodiscard]] inline NetworkEndpoint GetLocalEndpoint() const;

		[[nodiscard]] inline NetworkEndpoint GetRemoteEndpoint() const;

		int Start(const NetworkEndpoint &local, const NetworkEndpoint &remote); //TODO: define parameters

		void Close();

		uint16_t Send(std::span<uint8_t> data_to_send);

		[[nodiscard]] std::vector<uint8_t> Receive();

		[[nodiscard]] std::array<const PacketResult, sizeof(uint32_t) * 8> GetAcknowledge() const;
	};
}
#endif //RUDPLIB_CLIENT_HPP
