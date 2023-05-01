#include <winsock2.h>

#include "main.hpp"

uint8_t receiveBuffer[8192];
uint8_t sendBuffer[8192];

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::vector<uint8_t> expectedMemory{
        0x55, 0xAA, // Application ID
        0xAA, 0x55, // Sequence Number
        0x55, 0xAA, // Remote Sequence Number
        0xAA, 0x55, 0xAA, 0x55, // Acknowledge Bit Field
        0x00, 0x05, // Type
        0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, // Data
        0x64, 0xBD, 0xD2, 0x48, // CRC32
    };

    std::span<uint8_t> expectedSpan{expectedMemory};

    auto testPacket = reinterpret_cast<struct packet*>(malloc(sizeof(struct packet) + 8 + sizeof(uint32_t)));
    testPacket->dataLength = 8;
    testPacket->appId = 0x55AA;
    testPacket->sequenceNumber = 0xAA55;
    testPacket->ackSequenceNumber = 0x55AA;
    testPacket->ackBitfield = 0x55AA55AA;
    testPacket->type = 5;
    testPacket->data[0] = 0x55; testPacket->data[1] = 0xAA; testPacket->data[2] = 0x55; testPacket->data[3] = 0xAA;
    testPacket->data[4] = 0x55; testPacket->data[5] = 0xAA; testPacket->data[6] = 0x55; testPacket->data[7] = 0xAA;

    std::span<uint8_t> testPacketSpan{reinterpret_cast<uint8_t*>(&testPacket->appId), testPacket->size()};

    testPacket->appId = htons(testPacket->appId);
    testPacket->sequenceNumber = htons(testPacket->sequenceNumber);
    testPacket->ackSequenceNumber = htons(testPacket->ackSequenceNumber);
    testPacket->type = htons(testPacket->type);

    for(auto i{0}; i < expectedSpan.size(); i++)
    {
        if(expectedSpan[i] != testPacketSpan[i])
        {
            std::cout << "uhoh" << std::endl;
        }
    }

    std::span<uint8_t> testPacketData(testPacket->data, testPacket->dataLength);

    auto packetBufferOne = reinterpret_cast<uint8_t*>(&testPacket->appId);
    std::span<uint8_t> testSpanOne(packetBufferOne, testPacket->size());

    uint8_t testSerialize[64]{0};
    auto size = testPacket->serialize(std::span<uint8_t>(testSerialize, 64));

    auto deserializeTest = packet::deserialize(std::span<uint8_t>(testSerialize, size));
    std::span<uint8_t> deserializeTestData(deserializeTest->data, deserializeTest->dataLength);

    for(unsigned char & i : receiveBuffer)
    {
        i = (rand() % 256) - 1;
    }

    std::span<uint8_t> testSpan(receiveBuffer, 256);

    testSpan[8] = 42;

    struct sockaddr_in listenAddress = {};
    listenAddress.sin_family = AF_INET;
    listenAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    listenAddress.sin_port = htons(1337);

    std::vector<uint32_t> clientSockets();

    auto listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(listenSocket, (struct sockaddr*)&listenAddress, sizeof(listenAddress));
    listen(listenSocket, 5);

    while (true)
    {

    }

    return 0;
}

bool socket_isSocketPendingBytes(int socket, int timeoutSeconds)
{
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(socket, &rfd);

    struct timeval timeout =
            {
                    .tv_sec = timeoutSeconds,
                    .tv_usec = 0,
            };

    int ret = select(socket + 1, &rfd, NULL, NULL, &timeout);

    return FD_ISSET(socket, &rfd)? true : false;
}
