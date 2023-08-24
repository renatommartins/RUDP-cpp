#ifndef RUDPLIB_CLIENT_HPP
#define RUDPLIB_CLIENT_HPP

#include <cstdint>
#include <array>
#include <chrono>
#include <expected>
#include <mutex>
#include <queue>
#include <span>
#include <thread>
#include <vector>

#include "utils/time.hpp"

#include "ClientState.hpp"
#include "NetworkEndpoint.hpp"
#include "NetworkTranceiver.hpp"
#include "Packet.hpp"
#include "PacketResult.hpp"
#include "UdpTransceiver.hpp"


namespace rudp {
	enum class ClientMode {
		Threaded,
		Synchronous,
	};
	enum class ClientUpdateError {
		Disconnected,
		ForceClosed,
		InvalidMode,
		InvalidState,
		CouldntOpen,
	};
	class Client {
	private:
		std::jthread connection_thread;

		uint16_t application_id;
		std::shared_ptr<NetworkTransceiver> network_transceiver;
		ClientMode client_mode;
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

		rudp::utils::chrono::time_point_ms request_expire_time;
		rudp::utils::chrono::time_point_ms next_send_time;

		static void ConnectionThread(std::stop_token stop_token, Client* client);
		static std::expected<rudp::utils::chrono::time_point_ms, ClientUpdateError> ConnectionUpdate(Client *const client);
		static std::unique_ptr<const Packet> ReceivePacket(Client* client);
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
				ClientMode client_mode = ClientMode::Threaded,
				std::shared_ptr<NetworkTransceiver> network_transceiver = std::make_shared<UdpTransceiver>());

		[[nodiscard]] inline int GetAvailable() const;

		[[nodiscard]] int GetRtt() const;

		[[nodiscard]] inline ClientState GetState() const;

		[[nodiscard]] inline NetworkEndpoint GetLocalEndpoint() const;

		[[nodiscard]] inline NetworkEndpoint GetRemoteEndpoint() const;

		int Start(const NetworkEndpoint &local, const NetworkEndpoint &remote); //TODO: define parameters

		std::expected<rudp::utils::chrono::time_point_ms, ClientUpdateError> SynchronousUpdate();

		void Close();

		uint16_t Send(std::span<uint8_t> data_to_send);

		[[nodiscard]] std::vector<uint8_t> Receive();

		[[nodiscard]] std::array<const PacketResult, sizeof(uint32_t) * 8> GetAcknowledge() const;
	};
}
#endif //RUDPLIB_CLIENT_HPP
