#include "time.hpp"

namespace rudp::utils::chrono
{
	time_point_ms GetTimePointNowMs() {
		return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	}
}
