#include "util/time.hpp"
#include <ctime>
#include <iostream>
#include <string>
#include <chrono>

namespace mjon661 {
	std::string prettyTimestamp() {
		char buf[128];
		
		std::time_t t = std::time(nullptr);
		
		if(std::strftime(buf, 128, "%A %c %z", std::localtime(&t)))
			return std::string(buf);
		
		std::cerr << __FILE__ << " " << __LINE__ << ": strftime() failed\n";
		return std::to_string(t);
	}
	
	
	void Timer::start() {
		mStartTime = Clock::now();
	}
	
	void Timer::stop() {
		mEndTime = Clock::now();
		mStdChronoDuration = mEndTime - mStartTime;
	}
	
	double Timer::seconds() {
		return mStdChronoDuration.count();
	}
}
