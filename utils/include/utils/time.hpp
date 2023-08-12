#ifndef RUDPLIB_TIME_HPP
#define RUDPLIB_TIME_HPP

#include <chrono>
namespace rudp::utils::chrono {
	using time_point_ms =
			std::chrono::time_point<
					std::chrono::system_clock,
					std::chrono::milliseconds>;

	time_point_ms GetTimePointNowMs();
}

#endif //RUDPLIB_TIME_HPP
