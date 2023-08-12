#include <optional>

#include <gtest/gtest.h>

#include "rudplib/Packet.hpp"

#include "hello_test.hpp"

TEST(HelloTest, BasicAssertions) {
	EXPECT_STRCASENE("hello", "world");

	EXPECT_EQ(7 * 6, 42);
}

TEST(Packet, SerializeDeserialize) {
	std::vector<uint8_t> expectedMemory {
			0x55, 0xAA, // Application ID: 21930
			0xAA, 0x55, // Sequence Number: 43605
			0x55, 0xAA, // Remote Sequence Number: 21930
			0xAA, 0x55, 0xAA, 0x55, // Acknowledge Bit Field
			0x00, 0x05, // Type: 5
			0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, // Data
			0x64, 0xBD, 0xD2, 0x48, // CRC32: 1690161736
	};

	std::span<uint8_t> rawSpan{expectedMemory};
	auto ack = *reinterpret_cast<uint32_t*>(&expectedMemory[6]);
	auto dataVector = std::vector<uint8_t>{
		expectedMemory.begin() + 12,
		expectedMemory.end() - 4};

	auto createdPacket = rudp::Packet::CreatePacket(
			21930u,
			43605,
			21930,
			ack,
			rudp::PacketType::Data,
			std::optional<std::vector<uint8_t>>{dataVector});

	auto serializedPacket = rudp::Packet::Serialize(createdPacket);
	auto deserializedPacket = rudp::Packet::Deserialize(serializedPacket);
}