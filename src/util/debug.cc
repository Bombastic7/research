#include "util/debug.hpp"



namespace mjon661 {
	std::ofstream g_logDebugOfs;
	GblInitImpl<> g_gblInitImpl;
	
	bool debugCheckMemLimit() {
		struct sysinfo si;
		if(sysinfo(&si) != 0)
			logDebug("sysinfo failed.");
		else {
			logDebugStream()
				<< " freeram=" << si.freeram 
				<< " totalram=" << si.totalram
				<< " free=" << (double)si.freeram / si.totalram << "\n";
			
			if((double)si.freeram / si.totalram < 0.15)
				return true;
		}
		return false;
	}
}
