#include <cstdint>
#include <span>

#ifndef UTILS_CRC32_H
#define UTILS_CRC32_H

namespace rudp::utils::crc {
	uint32_t calculateCrc32(std::span<uint8_t const> buffer);
}

#endif //UTILS_CRC32_H
