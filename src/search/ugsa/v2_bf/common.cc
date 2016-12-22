#include "search/ugsa/v2_bf/common.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"

#include <cmath>


namespace mjon661 { namespace algorithm { namespace ugsav2_bf {


	UGSAConfig::UGSAConfig(Json const& j) {
		wt = j["wt"];
		wf = j["wf"];
		
		if(wt < 0 || wf < 0)
			throw ConfigException("Bad UGSA config");
	}
	
	Json UGSAConfig::report() const {
		Json j;
		j["wt"] = wt;
		j["wf"] = wf;
		return j;
	}


		
	void UGSABehaviour::reset() {
		mBaseDepthCount.fill(1);
		curExpTime = curExpd = 0;
		nextExpd = First_Resort_Expd;
		nUpdates = 0;
		mBF = 1;
		mMaxDepth = 0;
		phaseTimer.start();
	}
	
	void UGSABehaviour::update(unsigned pExpd) {
		phaseTimer.stop();
		phaseTimer.start();
		
		double avgBF = 0;
		
		for(unsigned i=1; i<=mMaxDepth; i++) {
			avgBF += log(mBaseDepthCount[i]) / log(i);
		}
		
		avgBF /= mMaxDepth;
		
		mBF = avgBF;
		

		curExpTime = phaseTimer.seconds() / (pExpd - curExpd);
		curExpd = pExpd;
		nextExpd = pExpd * Next_Resort_Fact;
	}
	
	void UGSABehaviour::putExpansion(unsigned pDepth) {
		mBaseDepthCount[pDepth]++;
		if(pDepth > mMaxDepth)
			mMaxDepth = pDepth;
	}
	
	bool UGSABehaviour::shouldUpdate(unsigned pExpd) const {
		return pExpd >= nextExpd;
	}
	
	double UGSABehaviour::remainingExpansions(unsigned L, unsigned pDepth) const {
		return pow(mBF, pDepth);
	}
	
	double UGSABehaviour::remainingTime(unsigned L, unsigned pDepth) const {
		return remainingExpansions(L, pDepth) * getExpTime();
	}
	
	double UGSABehaviour::getExpTime() const {
		return curExpTime;
	}
	
	Json UGSABehaviour::report() const {
		Json j;
		j["curBF"] = mBF;
		j["maxDepth"] = mMaxDepth;
		j["curExpTime"] = curExpTime;
		j["curExpd"] = curExpd;
		j["nextExpd"] = nextExpd;
		j["nUpdate"] = nUpdates;
		
		std::string distrStr("");
		
		for(unsigned i=0; i<=mMaxDepth; i++)
			distrStr += std::to_string(i) + ":" + std::to_string(mBaseDepthCount[i]) + "  ";
			
		j["distrStr"] = distrStr;
		
		return j;
	}
	
	UGSABehaviour::UGSABehaviour() {
		reset();
	}


	
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
