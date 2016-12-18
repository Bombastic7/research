#pragma once

#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/time.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav1 {
	
	using Util_t = double;
		
	struct UGSAConfig {
		UGSAConfig(Json const& j) {
			wt = j["wt"];
			wf = j["wf"];
			
			if(wt < 0 || wf < 0)
				throw ConfigException("Bad UGSA config");
		}
		
		double wt, wf;
	};
	
	
	struct UGSABehaviour {
		static const unsigned First_Resort_Expd = 128;
		static const unsigned Next_Resort_Fact = 2;
		
		
		void reset() {
			curDistCoeff = curExpDelay = curExpTime = curExpd = 0;
			nextExpDelay_acc = 0;
			nextExpd = First_Resort_Expd;
			phaseTimer.start();
		}
		
		void update(unsigned pExpd) {
			phaseTimer.stop();
			phaseTimer.start();
			
			curExpDelay = nextExpDelay_acc / (pExpd - curExpd);
			curExpTime = phaseTimer.seconds() / (pExpd - curExpd);
			curDistCoeff = curExpDelay * curExpTime;
			curExpd = pExpd;
			
			nextExpDelay_acc = 0;
			nextExpd = pExpd * Next_Resort_Fact;
		}
		
		void putExpDelay(unsigned pDelay) {
			nextExpDelay_acc += pDelay;
		}
		
		bool shouldUpdate(unsigned pExpd) {
			return pExpd >= nextExpd;
		}
		
		double remainingExpansions(unsigned pDepth) {
			return curExpDelay * pDepth;
		}
		
		double abtDist(unsigned pDepth) {
			return pDepth;
		}
		
		double getExpTime() {
			return curExpTime;
		}
		
		Json report() {
			Json j;
			j["curExpDelay"] = curExpDelay;
			j["curExpTime"] = curExpTime;
			j["curExpd"] = curExpd;
			j["nextExpd"] = nextExpd;
			return j;
		}
		
		
		double 		curDistCoeff;
		unsigned 	curExpDelay;
		double 		curExpTime;
		unsigned 	curExpd;

		Timer 		phaseTimer;
		double 		nextExpDelay_acc;
		unsigned 	nextExpd;
	};
		
}}}
