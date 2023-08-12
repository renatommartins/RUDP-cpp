#include <cstdint>

#include "portability.hpp"

namespace rudp::utils::portability{
	// https://stackoverflow.com/questions/1001307/detecting-endianness-programmatically-in-a-c-program

	bool is_big_endian() {
		union {
			uint32_t uint32bit;
			uint8_t uint8bitArray[4];
		} const testBinary{0x01020304};

		return testBinary.uint8bitArray[0] == 0x01;
	}
}