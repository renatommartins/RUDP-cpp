#ifndef RUDPLIB_PACKET_HPP
#define RUDPLIB_PACKET_HPP

#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <memory>

#include "PacketType.hpp"

namespace rudp {
#pragma pack(push, 1)
	struct Packet {
		uint16_t app_id;
		uint16_t sequence_number;
		uint16_t ack_sequence_number;
		union {
			uint32_t ack;
			uint8_t ack_bytes[4];
			struct {
				bool ack1: 1; bool ack2: 1; bool ack3: 1; bool ack4: 1;
				bool ack5: 1; bool ack6: 1; bool ack7: 1; bool ack8: 1;

				bool ack9: 1; bool ack10: 1; bool ack11: 1; bool ack12: 1;
				bool ack13: 1; bool ack14: 1; bool ack15: 1; bool ack16: 1;

				bool ack17: 1; bool ack18: 1; bool ack19: 1; bool ack20: 1;
				bool ack21: 1; bool ack22: 1; bool ack23: 1; bool ack24: 1;

				bool ack25: 1; bool ack26: 1; bool ack27: 1; bool ack28: 1;
				bool ack29: 1; bool ack30: 1; bool ack31: 1; bool ack32: 1;
			} bits;
		};
		union {
			PacketType type;
			uint16_t type_int;
		};
		uint16_t data_length;
		uint8_t data[];
		// CRC32 comes after the data bytes.

		static constexpr size_t kMinPacketSize =
				sizeof(app_id) +
				sizeof(sequence_number) +
				sizeof(ack_sequence_number) +
				sizeof(ack_bytes) +
				sizeof(type) +
				sizeof(data_length);

		[[nodiscard]] static std::unique_ptr<const Packet> CreatePacket(
				uint16_t app_id,
				uint16_t sequence_number,
				uint16_t ack_sequence_number,
				uint32_t ack,
				PacketType type,
				std::optional<std::vector<uint8_t>> data);

		[[nodiscard]] static std::unique_ptr<const Packet> Deserialize(std::span<uint8_t> buffer);

		[[nodiscard]] static std::vector<uint8_t> Serialize(std::unique_ptr<const Packet> &packet);

		[[nodiscard]] size_t Size() const;
	};
#pragma pack(pop)
}

#endif //RUDPLIB_PACKET_HPP
