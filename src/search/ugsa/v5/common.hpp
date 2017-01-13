#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include "structs/simple_hashmap.hpp"
#include "util/json.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav5 {	


	template<typename = void>
	struct SearchBehaviour {
		
		
		SearchBehaviour(Json const& jConfig) {
			reset();
		}
		
		void informExpansion(unsigned pLvl) {
			mLevelCounts[pLvl].val++;
			mDirty = true;
		}
		
		void gethbf() {
			if(mDirty)
				computehbf();
			
			return mCachedBF;
		}
		
		void reset() {
			mLevelCounts.clear();
			mCachedBF = 0;
			mDirty = false;
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
			
			mCachedBF = acc / (bins.size());
			mDirty = false;
		}
		
		
		
		
		SimpleHashMap<unsigned, unsigned, 10000> mLevelCounts;
		double mCachedBF;
		bool mDirty;
		
	};
}}}
