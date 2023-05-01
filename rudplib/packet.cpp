#include <cstdlib>
#include <cstring>

#if WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "utils/crc32.hpp"
#include "utils/portability.hpp"

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

    memcpy(&newPacket->appId, pBuffer, buffer.size() - sizeof(uint32_t));
    newPacket->dataLength = buffer.size() - sizeof(struct packet);

    if(!is_big_endian())
    {
        newPacket->appId = ntohs(newPacket->appId);
        newPacket->sequenceNumber = ntohs(newPacket->sequenceNumber);
        newPacket->ackSequenceNumber = ntohs(newPacket->ackSequenceNumber);
        newPacket->type = ntohs(newPacket->type);
    }

    return newPacket;
}

size_t packet::serialize(std::span<uint8_t> buffer) {
    // Check if the buffer is large enough to contain the serialized packet
    if (buffer.size() < size()) {
        return -1;
    }

    auto is_little_endian = !is_big_endian();
    uint8_t* pBuffer = buffer.data();
    size_t packetSize = size();

    if(is_little_endian)
    {
        appId = htons(appId);
        sequenceNumber = htons(sequenceNumber);
        ackSequenceNumber = htons(ackSequenceNumber);
        type = htons(type);
    }

    // copy packet fields to buffer
    memcpy(pBuffer, reinterpret_cast<uint8_t*>(&appId), packetSize);

    // Calculate and append crc32 to buffer
    uint32_t crc = calculateCrc32(std::span<uint8_t>(buffer.data(), packetSize - sizeof(uint32_t)));
    *reinterpret_cast<uint32_t*>(pBuffer + packetSize - sizeof(uint32_t)) =
            is_little_endian? htonl(crc) : crc;

    if(is_little_endian)
    {
        appId = ntohs(appId);
        sequenceNumber = ntohs(sequenceNumber);
        ackSequenceNumber = ntohs(ackSequenceNumber);
        type = ntohs(type);
    }

    return packetSize;
}

size_t packet::size() {
    return sizeof(appId) +
           sizeof(sequenceNumber) +
           sizeof(ackSequenceNumber) +
           sizeof(ackBytes) +
           sizeof(type) +
           sizeof(uint8_t) * dataLength +
           sizeof(uint32_t); // crc32 at the end
}
