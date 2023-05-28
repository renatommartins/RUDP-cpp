#ifndef RUDPLIB_PACKET_HPP
#define RUDPLIB_PACKET_HPP

#include <cstdint>
#include <span>

#pragma pack(push, 1)
struct packet
{
    size_t dataLength;
    uint16_t appId;
    uint16_t sequenceNumber;
    uint16_t ackSequenceNumber;
    union {
        uint8_t ackBytes[4];
//TODO: create class for bit field with indexer operator overload
        struct {
            bool ack1 : 1; bool ack2 : 1; bool ack3 : 1; bool ack4 : 1;
            bool ack5 : 1; bool ack6 : 1; bool ack7 : 1; bool ack8 : 1;

            bool ack9 : 1; bool ack10 : 1; bool ack11 : 1; bool ack12 : 1;
            bool ack13 : 1; bool ack14 : 1; bool ack15 : 1; bool ack16 : 1;

            bool ack17 : 1; bool ack18 : 1; bool ack19 : 1; bool ack20 : 1;
            bool ack21 : 1; bool ack22 : 1; bool ack23 : 1; bool ack24 : 1;

            bool ack25 : 1; bool ack26 : 1; bool ack27 : 1; bool ack28 : 1;
            bool ack29 : 1; bool ack30 : 1; bool ack31 : 1; bool ack32 : 1;
        } bits;
    };
//TODO: Create class enum for type
    uint16_t type;
    uint8_t data[];

    static struct packet* deserialize(std::span<uint8_t> buffer);
    size_t serialize(std::span<std::uint8_t> buffer);
    size_t size();
};
#pragma pack(pop)

#endif //RUDPLIB_PACKET_HPP

