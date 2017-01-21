#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include "structs/simple_hashmap.hpp"
#include "util/json.hpp"
#include "search/ugsa/v5/abtcorr.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav5 {	

	using ucost_t = unsigned;
	
	struct SolValues {
		unsigned u, g;
		unsigned depth;
	};
	
	enum struct HeuristicModes {
		Min_Cost,
		Min_Dist,
		Min_Cost_Or_Dist,
		Min_Cost_And_Dist
	};

	/*
	template<typename = void>
	struct SearchBehaviour : public AbtCorrection<> {
		
		
		SearchBehaviour(Json const& jConfig) {
			reset();
		}
		
		void informExpansion(unsigned pLvl) {
			mLevelCounts[mExpd + pLvl].val++;
			mDirty = true;
			mExpd++;
		}
		

		double gethbf() {
			if(mDirty)
				computehbf();
			
			return mCachedBF;
		}
		
		void reset() {
			AbtCorrection<>::reset();
			mLevelCounts.clear();
			mCachedBF = 0;
			mDirty = false;
			mExpd = 0;
		}
		
		Json report() {
			
			Json jReport;
			jReport["hbf"] = gethbf();
			jReport["cost comp"] = this->getCostComp();
			jReport["dist comp"] = this->getDistComp();
			jReport["comp samples"] = this->getNsamples();
			jReport["expd"] = mExpd;
			
			Json jflvls = {};
			
			std::vector<unsigned> flvls = mLevelCounts.orderedKeys();
					
			for(unsigned i=0; i<flvls.size(); i++) {
				Json j = {{"lvl", flvls[i]}, {"count", mLevelCounts[flvls[i]].val}};
				jflvls.push_back(j);
			}
			
			//jReport["flvls"] = jflvls;
			
			return jReport;
		}
		
		private:
		
		void computehbf() {
			std::vector<unsigned> flvls = mLevelCounts.orderedKeys();
			
			unsigned f_ref = flvls.at(0);
			unsigned cnt_ref = mLevelCounts[f_ref].val;
			
			double acc = 1;
			
			for(unsigned i=1; i<flvls.size(); i++) {
				
				unsigned	f =		flvls[i];
				unsigned	cnt =	mLevelCounts[f].val;
				
				double 		r = 	(double) cnt / cnt_ref;
				unsigned 	exp = 	1.0 / (f - f_ref);
				
				acc += std::pow(r, exp);
			}
			
			mCachedBF = acc / (flvls.size());
			mDirty = false;
		}
		
		
		
		
		SimpleHashMap<unsigned, unsigned, 10000> mLevelCounts;
		double mCachedBF;
		bool mDirty;
		unsigned mExpd;
		
	};
	*/
}}}
