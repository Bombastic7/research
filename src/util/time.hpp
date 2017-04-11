#pragma once

#include <ctime>
#include <chrono>
#include <iostream>
#include <string>
#include <ctime>


namespace mjon661 {
	std::string prettyTimestamp();
	
	struct WallTimer {
		using Clock = std::chrono::high_resolution_clock;
		using StdChronoDuration = std::chrono::duration<double>;
		
		void start();
		
		void stop();
		
		double seconds();
		
		Clock::time_point mStartTime, mEndTime;
		StdChronoDuration mStdChronoDuration;
	};
	
	
	struct CpuTimer {
		void start();
		void stop();
		double seconds();
		
		std::clock_t mStartTime, mEndTime, mDur;
	};
	
	
	struct Timer {
		void start();
		void stop();
		double wallSeconds();
		double cpuSeconds();
		
		WallTimer mWallTimer;
		CpuTimer mCpuTimer;
	};
}
