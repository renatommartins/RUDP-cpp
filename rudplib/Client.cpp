#include <optional>
#include <thread>
#include <vector>

#include "Client.hpp"
#include "Packet.hpp"

using namespace std::chrono_literals;

namespace rudp {
	Client::Client(uint16_t application_id) :
		application_id{application_id},
		state{ClientState::Disconnected},
		receive_queue{},
		send_queue{},
		rtt_window{0},
		current_rtt{0},
		local_endpoint{0},
		remote_endpoint{0}
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

	sockaddr Client::GetLocalEndpoint() const {
		return local_endpoint;
	}

	sockaddr Client::GetRemoteEndpoint() const {
		return remote_endpoint;
	}

	/*int Client::Bind(const sockaddr &endpoint) {
		switch(endpoint.sa_family) {
			case AF_INET:
			case AF_INET6:
				break;
			default:
				return -1;
		}

		client_socket = socket(endpoint.sa_family, SOCK_DGRAM, IPPROTO_UDP);

		auto bind_result = bind(client_socket, &endpoint, sizeof(endpoint));
		if(bind_result == SOCKET_ERROR)
			return WSAGetLastError();

		return bind_result;
	}*/

	int Client::ConnectAndStart(const sockaddr &local, const sockaddr &remote) {
		local_endpoint = local;
		remote_endpoint = remote;

		connection_thread = std::jthread{ConnectionUpdate, this};

		//TODO: do error checking in open result
		//TODO: start connection handshake

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
		/*rudp::utils::chrono::time_point_ms now_ms = rudp::utils::chrono::GetTimePointNowMs();
		if (now_ms < next_time_point)
			return next_time_point - now_ms;*/

		const auto& application_id = client->application_id;
		auto& local_endpoint = client->local_endpoint;
		auto& remote_endpoint = client->remote_endpoint;

		auto& network_transceiver = client->network_transceiver;
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
			auto connection_packet = Packet::CreatePacket(
				application_id,
				next_sequence_number++,
				0,
				0,
				PacketType::ConnectionRequest,
				std::nullopt);

			auto serialized_connection_packet =
				Packet::Serialize(connection_packet);

			network_transceiver->Transmit(serialized_connection_packet);
			auto request_expire_time =
				rudp::utils::chrono::GetTimePointNowMs() + 1000ms;
			while (!stop_token.stop_requested()) {
				if(request_expire_time < rudp::utils::chrono::GetTimePointNowMs()){ return; }

				if (network_transceiver->GetAvailable() == 0) {
					std::this_thread::yield();
					continue;
				}

				auto connection_reply_packet = ReceivePacket(client);
				if (connection_reply_packet == nullptr) { return; }
				if (connection_reply_packet->type != PacketType::ConnectionAccept){ return; }

				break;
			}
		}

		auto next_send_time = rudp::utils::chrono::GetTimePointNowMs();
		while(!stop_token.stop_requested()) {
			while (network_transceiver->GetAvailable() > 0) {
				auto received_packet = ReceivePacket(client);

				switch (received_packet->type) {
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
						break;
					}
					default:
						break;
				}
			}

			if(next_send_time > rudp::utils::chrono::GetTimePointNowMs()) {
				std::this_thread::sleep_for(10ms);
				continue;
			}
			next_send_time = rudp::utils::chrono::GetTimePointNowMs() + 50ms;

			auto send_sequence_number = next_sequence_number++;
			std::unique_ptr<const Packet> send_packet = nullptr;

			if(!send_queue.empty()) {
				auto send_packet_data = std::vector<uint8_t>{0};

				while(!send_queue.empty()) {
					auto data = send_queue.front();
					send_queue.pop();

					send_packet_data.insert(send_packet_data.end(), data.begin(), data.end());
				}

				send_packet = Packet::CreatePacket(
					application_id,
					send_sequence_number,
					last_remote_sequence_number,
					remote_acknowledges,
					PacketType::Data,
					std::optional<std::vector<uint8_t>>{send_packet_data});
			}
			else if(false) { //TODO: set boolean when closing connection to send disconnection notify

			}
			else {
				send_packet = Packet::CreatePacket(
					application_id,
					send_sequence_number,
					last_remote_sequence_number,
					remote_acknowledges,
					PacketType::KeepAlive,
					std::nullopt);
			}

			auto send_data = Packet::Serialize(send_packet);
			network_transceiver->Transmit(send_data);

			std::this_thread::sleep_for(10ms);
		}

		{
			auto send_sequence_number = next_sequence_number++;
			auto disconnect_packet = Packet::CreatePacket(
				application_id,
				send_sequence_number,
				last_remote_sequence_number,
				remote_acknowledges,
				PacketType::DisconnectionNotify,
				std::nullopt);

			auto send_data = Packet::Serialize(disconnect_packet);
			network_transceiver->Transmit(send_data);
		}

		auto close_result = network_transceiver->Close();

		/*last_time_point = next_time_point;
		next_time_point = next_time_point + 20ms; //TODO: make update rate changeable
		return 20ms;*/
	}

	std::unique_ptr<const Packet> Client::ReceivePacket(Client* const client) {
		const auto& application_id = client->application_id;

		auto& network_transceiver = client->network_transceiver;

		auto& last_sequence_number_acknowledged = client->last_sequence_number_acknowledged;
		auto& local_acknowledges = client->local_acknowledges;
		auto& last_remote_sequence_number = client->last_remote_sequence_number;
		auto& remote_acknowledges = client->remote_acknowledges;

		auto received_data = network_transceiver->Receive();

		auto data_span = std::span{received_data};
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
}
