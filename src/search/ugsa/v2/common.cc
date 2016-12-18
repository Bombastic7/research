#include "search/ugsa/v2/common.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav2 {


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
		curDistCoeff = curExpDelay = curExpTime = curExpd = 0;
		nextExpDelay_acc = 0;
		nextExpd = First_Resort_Expd;
		nUpdates = 0;
		phaseTimer.start();
	}
	
	void UGSABehaviour::update(unsigned pExpd) {
		phaseTimer.stop();
		phaseTimer.start();
		
		curExpDelay = nextExpDelay_acc / (pExpd - curExpd);
		curExpTime = phaseTimer.seconds() / (pExpd - curExpd);
		curDistCoeff = curExpDelay * curExpTime;
		curExpd = pExpd;
		
		nextExpDelay_acc = 0;
		nextExpd = pExpd * Next_Resort_Fact;
	}
	
	void UGSABehaviour::putExpDelay(unsigned pDelay) {
		nextExpDelay_acc += pDelay;
	}
	
	bool UGSABehaviour::shouldUpdate(unsigned pExpd) const {
		return pExpd >= nextExpd;
	}
	
	double UGSABehaviour::remainingExpansions(unsigned L, unsigned pDepth) const {
		return curExpDelay * pDepth;
	}
	
	double UGSABehaviour::remainingTime(unsigned L, unsigned pDepth) const {
		return remainingExpansions(L, pDepth) * getExpTime();
	}
	
	double UGSABehaviour::getExpTime() const {
		return curExpTime;
	}
	
	Json UGSABehaviour::report() const {
		Json j;
		j["curExpDelay"] = curExpDelay;
		j["curExpTime"] = curExpTime;
		j["curExpd"] = curExpd;
		j["nextExpd"] = nextExpd;
		j["nUpdate"] = nUpdates;
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
