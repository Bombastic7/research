#pragma once

#include <ctime>
#include <chrono>
#include <iostream>
#include <string>

namespace mjon661 {
	std::string prettyTimestamp();
	
	struct Timer {
		using Clock = std::chrono::high_resolution_clock;
		using StdChronoDuration = std::chrono::duration<double>;
		
		void start();
		
		void stop();
		
		double seconds();
		
		Clock::time_point mStartTime, mEndTime;
		StdChronoDuration mStdChronoDuration;
	};
}
