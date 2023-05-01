#include <cstdint>
#include <span>

#ifndef UTILS_CRC32_H
#define UTILS_CRC32_H

uint32_t calculateCrc32(const std::span<uint8_t> buffer);

#endif //UTILS_CRC32_H
