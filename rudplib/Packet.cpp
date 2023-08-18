#include <cstdlib>
#include <cstring>

#if WIN32 || WIN64
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "utils/crc32.hpp"

#include "Packet.hpp"

namespace rudp {
	std::unique_ptr<const Packet> Packet::Deserialize(const std::span<uint8_t> buffer) {
		auto expected_crc32 = utils::crc::calculateCrc32(std::span<uint8_t>(buffer.data(), buffer.size() - sizeof(uint32_t)));

		uint32_t received_crc32 = *reinterpret_cast<const uint32_t *>(buffer.data() + buffer.size() - sizeof(uint32_t));
		received_crc32 = ntohl(received_crc32);

		if (expected_crc32 != received_crc32)
			return nullptr;

		auto packetMemory = malloc(buffer.size() - sizeof(uint32_t));
		auto packet = std::unique_ptr<Packet>(reinterpret_cast<Packet *>(packetMemory));

		memcpy(packetMemory, buffer.data(), buffer.size() - sizeof(uint32_t));

		packet->app_id = ntohs(packet->app_id);
		packet->sequence_number = ntohs(packet->sequence_number);
		packet->ack_sequence_number = ntohs(packet->ack_sequence_number);
		packet->type_int = ntohs(packet->type_int);
		packet->data_length = ntohs(packet->data_length);

		return packet;
	}

	std::vector<uint8_t> Packet::Serialize(std::unique_ptr<const Packet> &packet) {
		auto returnVector = std::vector<uint8_t>(packet->Size() + sizeof(uint32_t));

		auto serialized_packet = reinterpret_cast<Packet *>(returnVector.data());
		serialized_packet->app_id = htons(packet->app_id);
		serialized_packet->sequence_number = htons(packet->sequence_number);
		serialized_packet->ack_sequence_number = htons(packet->ack_sequence_number);
		serialized_packet->ack = packet->ack;
		serialized_packet->type_int = htons(packet->type_int);
		serialized_packet->data_length = htons(packet->data_length);
		memcpy(serialized_packet->data, packet->data, packet->data_length);

		auto crc = htonl(
				utils::crc::calculateCrc32(std::span<uint8_t>(returnVector.data(), returnVector.size() - sizeof(uint32_t))));

		memcpy(&returnVector[returnVector.size() - sizeof(uint32_t)], &crc, sizeof(uint32_t));

		return returnVector;
	}

	size_t Packet::Size() const {
		return kMinPacketSize + sizeof(uint8_t) * data_length; // crc32 at the end
	}

	std::unique_ptr<const Packet> Packet::CreatePacket(
			uint16_t app_id,
			uint16_t sequence_number,
			uint16_t ack_sequence_number,
			uint32_t ack,
			PacketType type,
			std::optional<std::vector<uint8_t>> data) {
		auto packetSize = kMinPacketSize + data.value().size();
		auto packet = std::unique_ptr<Packet>(reinterpret_cast<Packet *>(malloc(packetSize)));

		packet->app_id = app_id;
		packet->sequence_number = sequence_number;
		packet->ack_sequence_number = ack_sequence_number;
		packet->ack = ack;
		packet->type = type;

		if (data.has_value()) {
			packet->data_length = static_cast<uint16_t>(data.value().size());
			memcpy(packet->data, data.value().data(), data.value().size());
		}

		return packet;
	}
}
