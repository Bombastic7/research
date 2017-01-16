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
	struct AbtCorrection {
		
		AbtCorrection() {
			reset();
		}
		
		void setInitNodeValues(unsigned pHval, unsigned pDist) {
			mInitHval = pHval;
			mInitDist = pDist;
			mInitValsSet = true;
		}
		
		
		void informAbtSearch(unsigned pG, unsigned pH, unsigned pDepth, unsigned pDist) {
			slow_assert(mInitValsSet);
			
			if(mInitHval == pH)
				mCostCompAcc += 1;
			else {
				double v = (double) pG / (mInitHval - pH);
				slow_assert(v >= 0 && std::isfinite(v));
				
				mCostCompAcc += v;
			}
			
			if(mInitDist == pDist)
				mDistCompAcc += 1;
			else {
				double v = (double) pDepth / (mInitDist - pDist);
				slow_assert(v >= 0 && std::isfinite(v));
				
				mDistCompAcc += v;
			}
			
			mNsamples++;
		}
		
		double getCostComp() {
			return mCostCompAcc / mNsamples;
		}
		
		double getDistComp() {
			return mDistCompAcc / mNsamples;
		}
		
		unsigned getNsamples() {
			return mNsamples;
		}
		
		void reset() {
			mInitValsSet = false;
			mCostCompAcc = 0;
			mDistCompAcc = 0;
			mNsamples = 0;
		}
		
		private:
		
		unsigned mInitHval;
		unsigned mInitDist;
		bool mInitValsSet;
		
		double mCostCompAcc;
		double mDistCompAcc;
		unsigned mNsamples;
	};
}}}
