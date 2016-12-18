#include "search/hastar/generic/common.hpp"



namespace mjon661 { namespace algorithm { namespace hastargeneric {	


	AbtStatsCounter::AbtStatsCounter() :
		nGlobalSearches(0)
	{}
	
	unsigned AbtStatsCounter::getGlobalN() {
		return nGlobalSearches++;
	}
	
	void AbtStatsCounter::reset() {
		nGlobalSearches = 0;
	}

}}}
