#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include <type_traits>
#include <numeric_limits>

#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/time.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {	


	template<typename Cost, unsigned F_Range>
	struct HeuristicBF {
		
		static_assert(std::is_integral(Cost), "");
		

		void informNodeExpansion(Cost pfval) {
			slow_assert(pfval > 0 && pfval < F_Range);
			mLevelCount[(unsigned)pfval]++;	
		};
		
		
		//Memoise?
		flt_t computeHBF(Cost pFa, Cost pFb) {
			slow_assert(pFa >= 0 && pFa < F_Range && pFb > 0 && pFb < F_Range);
			slow_assert(pFa != pFb && pFa != 0);
			
			flt_t r = (flt_t)pFb / pFa;
			flt_t szDifRecip = 1.0 / (mLevelCount[pFb] - mLevelCount[pFa]);
			
			flt_t bf = std::pow(r, szDifRecip);
			return bf;
		}
		
		flt_t computeHBF() {
			flt_t acc = 0;
			unsigned samples = 0, i, j;
			
			for(; mLevelCount[i] == 0 && i < F_Range; i++) ;
			
			slow_assert(i != F_Range);
			
			j = i + 1;
			
			for(; mLevelCount[j] == 0 && j < F_Range; j++) ;
			
			if(j == F_Range)
				return 0;
			
			
			
			while(j < F_Range) {
				
				acc += computeHBF(i, j);
				samples++;
				
				i = j;
				j++:
				
				for(; mLevelCount[j] == 0 && j < F_Range; j++) ;
			}
			
			return acc / samples;
		}
		

		
		void reset() {
			mLevelCount.fill(0);
		}
		
		
		HeuristicBF() {
			reset();
		}

		std::array<unsigned, F_Range> mLevelCount;
		
	};


	
	template<typename BaseDomain, unsigned Bound>
	struct UGSABehaviour_B : public HeuristicBF<typename BaseDomain::Cost> {

		UGSABehaviour_B(unsigned pPref) :
			mPref(kPref)
		{
			reset();
		}
		
		unsigned compute_singleTree(unsigned pLvl, Cost pG) {
			slow_assert(pLvl == 1);
			
			flt_t remExp = std::pow(this->computeHBF(), pG); //<- dist correction?
			slow_assert((unsigned)remExp < std::numeric_limits<unsigned>::max() / 10); //10 is arbitrary.
			
			return pG + mPref * remExp;
		}
		
		unsigned compute_AllFrontier(unsigned pLvl, Cost pG, unsigned pSz) {
			return compute_singleTree(pLvl, pG) * pSz;
		}
		
		void reset() {
			HeuristicBF<typename BaseDomain::Cost>::reset();
		}
		
		Json report() {
			Json j;
			j["hbf"] = this->computeHBF();
			return j;
		}
	};
}}}
