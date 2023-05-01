#include <winsock2.h>
#include <cstring>

#include "utils/crc32.hpp"

#include "packet.hpp"

struct packet* packet::deserialize(std::span<uint8_t> buffer) {
    // Calculate the expected crc32 of the data
    uint32_t expected_crc32 = calculateCrc32(std::span<uint8_t>(buffer.data(), buffer.size() - sizeof(uint32_t)));

    // Convert the last 4 bytes to host byte order for crc32 validation
    uint32_t received_crc32;
    memcpy(&received_crc32, buffer.data() + buffer.size() - sizeof(uint32_t), sizeof(uint32_t));
    received_crc32 = ntohl(received_crc32);

    // Validate crc32
    if (expected_crc32 != received_crc32) {
        return nullptr;
    }

    // Allocate memory for packet with variable size data array
    auto newPacket = static_cast<packet*>(malloc(buffer.size()));
    uint8_t* pBuffer = buffer.data();

    // Deserialize fields
    newPacket->appId = ntohs(*reinterpret_cast<const uint16_t*>(pBuffer));
    pBuffer += sizeof(packet::appId);

    newPacket->sequenceNumber = ntohs(*reinterpret_cast<const uint16_t*>(pBuffer));
    pBuffer += sizeof(packet::sequenceNumber);

    newPacket->ackSequenceNumber = ntohs(*reinterpret_cast<const uint16_t*>(pBuffer));
    pBuffer += sizeof(packet::ackSequenceNumber);

    newPacket->ackBitfield = ntohl(*reinterpret_cast<const uint32_t*>(pBuffer));
    pBuffer += sizeof(packet::ackBitfield);

    newPacket->type = ntohs(*reinterpret_cast<const uint16_t*>(pBuffer));
    pBuffer += sizeof(packet::type);

    // Copy data to packet
    newPacket->dataLength = buffer.size() - sizeof(struct packet);
    memcpy(newPacket->data, pBuffer, newPacket->dataLength);

    return newPacket;
}


size_t packet::serialize(std::span<uint8_t> buffer) {
    // Check if the buffer is large enough to contain the serialized packet
    if (buffer.size() < size()) {
        return -1;
    }

    uint8_t* pBuffer = buffer.data();

    // copy packet fields to buffer
    *reinterpret_cast<uint16_t*>(pBuffer) = htons(appId);
    pBuffer += sizeof(packet::appId);

    *reinterpret_cast<uint16_t*>(pBuffer) = htons(sequenceNumber);
    pBuffer += sizeof(packet::sequenceNumber);

    *reinterpret_cast<uint16_t*>(pBuffer) = htons(ackSequenceNumber);
    pBuffer += sizeof(packet::ackSequenceNumber);

    *reinterpret_cast<uint32_t*>(pBuffer) = htonl(ackBitfield);
    pBuffer += sizeof(packet::ackBitfield);

    *reinterpret_cast<uint16_t*>(pBuffer) = htons(type);
    pBuffer += sizeof(packet::type);

    // copy packet data to buffer
    std::memcpy(pBuffer, data, dataLength);
    pBuffer += dataLength;

    // Calculate and append crc32 to buffer
    uint32_t crc = calculateCrc32(std::span<uint8_t>(buffer.data(), size()- sizeof(uint32_t)));
    *reinterpret_cast<uint32_t*>(pBuffer) = htonl(crc);

    return size();
}

size_t packet::size() {
    return sizeof(appId) +
           sizeof(sequenceNumber) +
           sizeof(ackSequenceNumber) +
           sizeof(ackBitfield) +
           sizeof(type) +
           sizeof(uint8_t) * dataLength +
           sizeof(uint32_t); // crc32 at the end
}
