#include <chrono>
#include <optional>
#include <thread>
#include <vector>

#include "Client.hpp"
#include "Packet.hpp"

#include "utils/time.hpp"

using namespace std::chrono_literals;

namespace rudp {
	Client::Client(
		uint16_t application_id,
		std::shared_ptr<NetworkTransceiver> network_transceiver) :
		network_transceiver{std::move(network_transceiver)},
		application_id{application_id},
		state{ClientState::Disconnected},
		receive_queue{},
		send_queue{},
		rtt_window{0},
		current_rtt{0},
		local_endpoint{},
		remote_endpoint{}
	{}

	int Client::GetAvailable() const {
		return static_cast<int>(receive_queue.size());
	}

	int Client::GetRtt() const {
		int rtt_average{0};

		for(const auto rtt : rtt_window)
			rtt_average += rtt;

		return static_cast<int>(rtt_average / rtt_window.size());
	}

	ClientState Client::GetState() const {
		return state;
	}

	NetworkEndpoint Client::GetLocalEndpoint() const {
		return local_endpoint;
	}

	NetworkEndpoint Client::GetRemoteEndpoint() const {
		return remote_endpoint;
	}

	int Client::Start(const NetworkEndpoint &local, const NetworkEndpoint &remote) {
		local_endpoint = local;
		remote_endpoint = remote;

		connection_thread = std::jthread{ConnectionUpdate, this};

		return 0;
	}

	void Client::Close() {
		//TODO: send disconnection notification
		auto close_result = network_transceiver->Close();
		//TODO: error checking
	}

	uint16_t Client::Send(std::span<uint8_t> data_to_send) {
		auto data_vector = std::vector<uint8_t>(data_to_send.begin(), data_to_send.end());
		send_queue.push(data_vector);

		return static_cast<uint16_t>(data_to_send.size());
	}

	std::vector<uint8_t> Client::Receive() {
		auto data_vector = receive_queue.front();
		receive_queue.pop();

		return data_vector;
	}

	std::array<const PacketResult, sizeof(uint32_t) * 8> Client::GetAcknowledge() const {
		//TODO: return last 32 packets acknowledge state
		return std::array<const PacketResult, sizeof(uint32_t) * 8>{0};
	}

	void Client::ConnectionUpdate(const std::stop_token stop_token, Client* const client) {
		const auto& application_id = client->application_id;
		auto& local_endpoint = client->local_endpoint;
		auto& remote_endpoint = client->remote_endpoint;

		auto& network_transceiver = client->network_transceiver;
		auto& state = client->state;
		auto& receive_queue = client->receive_queue;
		auto& send_queue = client->send_queue;
		auto& current_rtt = client->current_rtt;

		auto& next_sequence_number = client->next_sequence_number;
		auto& last_sequence_number_acknowledged = client->last_sequence_number_acknowledged;
		auto& local_acknowledges = client->local_acknowledges;
		auto& last_remote_sequence_number = client->last_remote_sequence_number;
		auto& remote_acknowledges = client->remote_acknowledges;

		auto rtt_array_index = 0;
		auto rtt_window = std::array<int, 8>{0};

		{
			auto open_result = network_transceiver->Open(local_endpoint, remote_endpoint);

			if(open_result == OpenResult::ResourceNotAvailable)
				return;
		}

		{
			SendPacket(
				network_transceiver,
				application_id,
				next_sequence_number++,
				0,
				0,
				PacketType::ConnectionRequest,
				std::nullopt);

			state = ClientState::Connecting;
			auto request_expire_time =
				rudp::utils::chrono::GetTimePointNowMs() + 1000ms;


			while (!stop_token.stop_requested()) {
				if (request_expire_time < rudp::utils::chrono::GetTimePointNowMs()) { return; }

				if (network_transceiver->GetAvailable() == 0) {
					std::this_thread::yield();
					continue;
				}

				auto connection_reply_packet = ReceivePacket(client);

				if (connection_reply_packet == nullptr ||
				    connection_reply_packet->type != PacketType::ConnectionAccept) {
					state = ClientState::Disconnected;
					return;
				}

				state = ClientState::Connected;
				break;
			}
		}

		auto next_send_time = rudp::utils::chrono::GetTimePointNowMs();

		while(state == ClientState::Connected && !stop_token.stop_requested()) {
			while (network_transceiver->GetAvailable() > 0) {
				auto received_packet = ReceivePacket(client);

				switch (received_packet->type) {
					case PacketType::KeepAlive:
						break;
					case PacketType::Data: {
						auto packet_data = std::vector<uint8_t>{
								received_packet->data,
								received_packet->data + received_packet->data_length
						};
						receive_queue.emplace(packet_data);
						break;
					}
					case PacketType::DisconnectionNotify: {
						while(!send_queue.empty())
							send_queue.pop();
						network_transceiver->Close();
						state = ClientState::Disconnected;
						break;
					}
					default: {
						state = ClientState::ForceClose;
						auto force_close_packet = Packet::CreatePacket(
							application_id,
							next_sequence_number++,
							last_remote_sequence_number,
							remote_acknowledges,
							PacketType::DisconnectionNotify,
							std::nullopt);
						auto send_data = Packet::Serialize(force_close_packet);
						network_transceiver->Transmit(send_data);

						break;
					}
				}
			}

			if(next_send_time > rudp::utils::chrono::GetTimePointNowMs()) {
				std::this_thread::sleep_for(10ms);
				continue;
			}
			next_send_time = rudp::utils::chrono::GetTimePointNowMs() + 50ms;

			PacketType packet_type;
			std::optional<std::vector<uint8_t>> packet_data;

			if(!send_queue.empty()) {
				packet_type = PacketType::Data;
				auto send_packet_data = std::vector<uint8_t>{0};

				while(!send_queue.empty()) {
					auto data = send_queue.front();
					send_queue.pop();

					send_packet_data.insert(send_packet_data.end(), data.begin(), data.end());
				}

				packet_data = std::optional<std::vector<uint8_t>>{send_packet_data};
			}
			else {
				packet_type = PacketType::KeepAlive;
				packet_data = std::nullopt;
			}

			SendPacket(
				network_transceiver,
				application_id,
				next_sequence_number++,
				last_remote_sequence_number,
				remote_acknowledges,
				packet_type,
				packet_data);

			std::this_thread::sleep_for(10ms);
		}

		SendPacket(
			network_transceiver,
			application_id,
			next_sequence_number++,
			last_remote_sequence_number,
			remote_acknowledges,
			PacketType::DisconnectionNotify,
			std::nullopt
		);

		state = ClientState::Disconnected;

		auto close_result = network_transceiver->Close();
	}

	std::unique_ptr<const Packet> Client::ReceivePacket(Client* const client) {
		const auto& application_id = client->application_id;

		auto& network_transceiver = client->network_transceiver;

		auto& last_sequence_number_acknowledged = client->last_sequence_number_acknowledged;
		auto& local_acknowledges = client->local_acknowledges;
		auto& last_remote_sequence_number = client->last_remote_sequence_number;
		auto& remote_acknowledges = client->remote_acknowledges;

		auto received_data = network_transceiver->Receive();

		if (!received_data.has_value())
			return nullptr;

		auto data_span = std::span{received_data.value()};
		auto received_packet = Packet::Deserialize(data_span);

		if (received_packet == nullptr) { return nullptr; }
		if (received_packet->app_id != application_id) { return nullptr; }
		if (received_packet->sequence_number <= last_remote_sequence_number) { return nullptr; }

		auto remote_sequence_number_increment =
			received_packet->sequence_number - last_remote_sequence_number;
		last_remote_sequence_number = received_packet->sequence_number;
		remote_acknowledges >>= remote_sequence_number_increment;
		remote_acknowledges |= 1 << (sizeof(uint32_t) * 8 - remote_sequence_number_increment);

		last_sequence_number_acknowledged = received_packet->ack_sequence_number;
		local_acknowledges = received_packet->ack;

		return received_packet;
	}

	void Client::SendPacket(
		std::shared_ptr<NetworkTransceiver> &network_transceiver,
		uint16_t app_id,
		uint16_t sequence_number,
		uint16_t ack_sequence_number,
		uint32_t ack,
		PacketType type,
		std::optional<std::vector<uint8_t>> data) {
		auto packet = Packet::CreatePacket(
			app_id,
			sequence_number,
			ack_sequence_number,
			ack,
			type,
			std::move(data));

		auto send_data = Packet::Serialize(packet);

		network_transceiver->Transmit(send_data);
	}
}
