#include <chrono>
#include <optional>
#include <thread>
#include <vector>

#include "utils/time.hpp"

#include "Packet.hpp"

#include "Client.hpp"

using namespace std::chrono_literals;
using rudp::utils::chrono::GetTimePointNowMs;
using rudp::utils::chrono::time_point_ms;

namespace rudp {
	Client::Client(
		uint16_t application_id,
		ClientMode client_mode,
		std::shared_ptr<NetworkTransceiver> network_transceiver) :
		network_transceiver{std::move(network_transceiver)},
		application_id{application_id},
		client_mode{client_mode},
		state{ClientState::Disconnected},
		receive_queue{},
		send_queue{},
		rtt_window{0},
		current_rtt{0},
		local_endpoint{},
		remote_endpoint{},
		request_expire_time{},
		next_sequence_number{0},
		last_sequence_number_acknowledged{0xFFFF},
		local_acknowledges{0},
		last_remote_sequence_number{0xFFFF},
		remote_acknowledges{0}
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

		if(client_mode == ClientMode::Threaded)
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

	std::array<PacketResult, Client::kPacketAcknowledgeLength> Client::GetAcknowledge() const {
		std::array<PacketResult, kPacketAcknowledgeLength> return_value{0};

		return_value.at(0) = PacketResult {
			last_sequence_number_acknowledged,
			true,
		};

		int offset = 31;
		uint16_t upper_boundary = last_sequence_number_acknowledged-1;
		uint16_t lower_boundary = last_sequence_number_acknowledged - kPacketAcknowledgeLength + 1;
		for (uint16_t i = upper_boundary; i > lower_boundary; i--, offset++) {
			return_value.at(i + 1) = PacketResult{
				i,
				(local_acknowledges & (1 << offset)) != 0,
			};
		}
		return return_value;
	}

	std::expected<rudp::utils::chrono::time_point_ms, ClientUpdateError> Client::SynchronousUpdate() {
		if (client_mode != ClientMode::Synchronous)
			return std::unexpected(ClientUpdateError::InvalidMode);

		auto update_result = ConnectionUpdate(this);

		return update_result;
	}

	void Client::ConnectionThread(const std::stop_token &stop_token, Client* const client) {
		auto next_send_time = rudp::utils::chrono::GetTimePointNowMs();

		while(client->state == ClientState::Connected && !stop_token.stop_requested()) {
			auto update_result = ConnectionUpdate(client);

			if(!update_result.has_value())
				break;

			std::this_thread::sleep_until(update_result.value());
		}
	}

	std::expected<rudp::utils::chrono::time_point_ms, ClientUpdateError> Client::ConnectionUpdate(Client* const client) {
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

		auto& request_expire_time = client->request_expire_time;
		auto& next_send_time = client->next_send_time;

		auto rtt_array_index = 0;
		auto rtt_window = std::array<int, 8>{0};

		switch(state) {
			case ClientState::Disconnected: {
				auto open_result = network_transceiver->Open(local_endpoint, remote_endpoint);

				if (open_result == OpenResult::ResourceNotAvailable)
					return std::unexpected(ClientUpdateError::CouldntOpen);

				SendPacket(
					network_transceiver,
					application_id,
					next_sequence_number++,
					0,
					0,
					PacketType::ConnectionRequest,
					std::nullopt);

				state = ClientState::Connecting;

				request_expire_time =
					GetTimePointNowMs() + 1000ms; //TODO: make connection timeout configurable

				return GetTimePointNowMs() + 50ms; //TODO: should connection request update interval be the same as connection keep alive?
			}
			case ClientState::Connecting: {
				if (request_expire_time < GetTimePointNowMs()) {
					state = ClientState::Disconnected;
					return std::unexpected(ClientUpdateError::Disconnected);
				}

				if (!network_transceiver->IsDataAvailable())
					return GetTimePointNowMs() + 50ms;

				auto connection_reply_packet = ReceivePacket(client);

				if (connection_reply_packet == nullptr ||
				    connection_reply_packet->type != PacketType::ConnectionAccept) {

					state = ClientState::Disconnected;
					return std::unexpected(ClientUpdateError::Disconnected);
				}

				state = ClientState::Connected;
				return GetTimePointNowMs() + 20ms;
			}
			case ClientState::Connected: {
				while (network_transceiver->IsDataAvailable()) {
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
							return std::unexpected(ClientUpdateError::Disconnected);
						}
						default: {
							auto force_close_packet = Packet::CreatePacket(
								application_id,
								next_sequence_number++,
								last_remote_sequence_number,
								remote_acknowledges,
								PacketType::DisconnectionNotify,
								std::nullopt);
							auto send_data = Packet::Serialize(force_close_packet);
							network_transceiver->Transmit(send_data);

							state = ClientState::ForceClose;
							return std::unexpected(ClientUpdateError::ForceClosed);
						}
					}
				}

				if(next_send_time > GetTimePointNowMs())
					return next_send_time;

				next_send_time = GetTimePointNowMs() + 50ms;

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

				return next_send_time;
			}
			case ClientState::Disconnecting:{
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
				return std::unexpected(ClientUpdateError::Disconnected);
			}
			default:
				return std::unexpected(ClientUpdateError::InvalidState);
		}
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

	bool Client::CompareSequenceNumberGreaterThan(const uint16_t s1, const uint16_t s2) noexcept {
		return ((s1 > s2) && (s1 - s2 <= 0xFFFFui16 / 2)) ||
		       ((s1 < s2) && (s2 - s1 > 0xFFFFui16 / 2));;
	}
}
