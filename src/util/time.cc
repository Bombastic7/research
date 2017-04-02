#include "util/time.hpp"
#include <ctime>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>

namespace mjon661 {
	std::string prettyTimestamp() {
		char buf[128];
		
		std::time_t t = std::time(nullptr);
		
		if(std::strftime(buf, 128, "%A %c %z", std::localtime(&t)))
			return std::string(buf);
		
		std::cerr << __FILE__ << " " << __LINE__ << ": strftime() failed\n";
		return std::to_string(t);
	}
	
	
	void WallTimer::start() {
		mStartTime = Clock::now();
	}
	
	void WallTimer::stop() {
		mEndTime = Clock::now();
		mStdChronoDuration = mEndTime - mStartTime;
	}
	
	double WallTimer::seconds() {
		return mStdChronoDuration.count();
	}
	
	
	
	void CpuTimer::start() {
		mStartTime = std::clock();
	}
	
	void CpuTimer::stop() {
		mEndTime = std::clock();
	}
	
	double CpuTimer::seconds() {
		return (double)(mEndTime - mStartTime) / CLOCKS_PER_SEC;
	}
	
	
	void Timer::start() {
		mWallTimer.start();
		mCpuTimer.start();
	}
	
	void Timer::stop() {
		mWallTimer.stop();
		mCpuTimer.stop();
	}
	
	double Timer::wallSeconds() {
		return mWallTimer.seconds();
	}
	
	double Timer::cpuSeconds() {
		return mCpuTimer.seconds();
	}
	
}
