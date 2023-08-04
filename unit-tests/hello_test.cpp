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
	auto dataSpan = std::span<uint8_t>{&expectedMemory[12], 8};

	auto createdPacket = Packet::CreatePacket(
			21930u,
			43605,
			21930,
			ack,
			PacketType::Data,
			dataSpan);

	auto serializedPacket = Packet::Serialize(createdPacket);
	auto deserializedPacket = Packet::Deserialize(serializedPacket);
}