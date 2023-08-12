#include <iostream>
#include <vector>
#include <span>

#include "rudplib/Packet.hpp"

#include "main.hpp"

using std::cout;
using std::endl;

uint8_t receiveBuffer[8192];
uint8_t sendBuffer[8192];

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::vector<uint8_t> expectedMemory{
        0x55, 0xAA, // Application ID: 21930
        0xAA, 0x55, // Sequence Number: 43605
        0x55, 0xAA, // Remote Sequence Number: 21930
        0xAA, 0x55, 0xAA, 0x55, // Acknowledge Bit Field
        0x00, 0x05, // Type: 5
        0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, // Data
        0x64, 0xBD, 0xD2, 0x48, // CRC32: 1690161736
    };
    std::span<uint8_t> rawSpan{expectedMemory};

    auto testPacket = reinterpret_cast<struct Packet*>(malloc(sizeof(struct Packet) + 8 + sizeof(uint32_t)));
    testPacket->data_length = 8;
    testPacket->app_id = 0x55AA;
    testPacket->sequence_number = 0xAA55;
    testPacket->ack_sequence_number = 0x55AA;
    testPacket->ack_bytes[0] = 0xAA; testPacket->ack_bytes[1] = 0x55;
    testPacket->ack_bytes[2] = 0xAA; testPacket->ack_bytes[3] = 0x55;
    testPacket->type = 5;
    testPacket->data[0] = 0x55; testPacket->data[1] = 0xAA; testPacket->data[2] = 0x55; testPacket->data[3] = 0xAA;
    testPacket->data[4] = 0x55; testPacket->data[5] = 0xAA; testPacket->data[6] = 0x55; testPacket->data[7] = 0xAA;

    auto rawPacket{Packet::Deserialize(rawSpan)};

    uint8_t serializeBuffer[64]{0};
    auto serializedSize = testPacket->Serialize(std::span<uint8_t>(serializeBuffer, sizeof(serializeBuffer)));

    cout << std::hex;
    for(const auto &byte : rawSpan)
        cout << static_cast<int>(byte) << ':';
    cout << " - rawSpan" << endl;

    for(const auto &byte : std::span<uint8_t>(reinterpret_cast<uint8_t*>(&testPacket->app_id), testPacket->Size()))
        cout << static_cast<int>(byte) << ':';
    cout << " - testPacket (memory)" << endl;

    for(const auto &byte : std::span<uint8_t>(reinterpret_cast<uint8_t*>(&rawPacket->app_id), testPacket->Size()))
        cout << static_cast<int>(byte) << ':';
    cout << " - rawPacket (memory)" << endl;

    for(const auto &byte : std::span<uint8_t>(serializeBuffer, serializedSize))
        cout << static_cast<int>(byte) << ':';
    cout << " - serializeBuffer" << endl;

    cout<<std::dec;

    return 0;
}
