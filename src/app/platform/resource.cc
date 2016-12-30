#include "app/platform/resource.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>

#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/time.hpp"


namespace mjon661 {

	void sigHandler(int pSig) {
		if(pSig == SIGXCPU) {
			Json j;
			j["_result"] = "OOT";
			std::string msg = j.dump();
			write(1, msg.c_str(), msg.size());
		}
		exit(0);
	}
	
	static bool installSigHandler() {		
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		
		sa.sa_handler = sigHandler;
		
		if(sigaction(SIGXCPU, &sa, NULL) != 0)
			return false;

		return true;
	}
	
	static double setResourceLimit(int pResource, double pVal) {
		rlimit rl;
		int res = getrlimit(pResource, &rl);
		
		if(res != 0)
			return -1;
		
		if(rl.rlim_cur > pVal) {
			rl.rlim_cur = pVal;
			//rl.rlim_max = pVal + 10;
			res = setrlimit(pResource, &rl);
			
			if(res != 0)
				return -1;
		}
		
		return rl.rlim_cur;
	}

	double setCpuTimeLimit(double pSeconds) {
		if(!installSigHandler())
			return -1;
		return setResourceLimit(RLIMIT_CPU, pSeconds);
	}
	
	double setVirtMemLimit(double pMB) {
		return setResourceLimit(RLIMIT_AS, pMB*1000000)/1000000;
	}


	Json resourceReport() {
		rusage oRusage;
		int res = getrusage(RUSAGE_SELF, &oRusage);
		if(res != 0)
			logDebug(std::string(__FILE__ " ") + std::to_string(__LINE__) + " getrusage failed");

		double userTime = 0;
		double kernelTime = 0;
		double maxMem = 0;
		
		if(res == 0) {
			userTime = (double)oRusage.ru_utime.tv_sec + oRusage.ru_utime.tv_usec / 1e6;
			kernelTime = (double)oRusage.ru_stime.tv_sec + oRusage.ru_stime.tv_usec / 1e6;
			maxMem = oRusage.ru_maxrss / 1e3;
		}
		
		Json j;
		j["user time (s)"] = userTime;
		j["kernel time (s)"] = kernelTime;
		j["cputime"] = userTime + kernelTime;
		j["max rss (MB)"] = maxMem;
		
		return j;
	}


}
