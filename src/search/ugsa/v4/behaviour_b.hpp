#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <cmath>
#include <string>
#include <type_traits>
#include <limits>


#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/time.hpp"

#include "search/ugsa/v4/defs.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {	


	template<typename Cost>
	struct HeuristicBF {
		
		static_assert(std::is_integral<Cost>::value, "");

		void informNodeExpansion(Cost, Cost pfval) {
			mLevelCount[(unsigned)pfval]++;
			
			if(!mInitFvalSet) {
				mInitFvalSet = true;
				mInitNodeFval = pfval;
			}
		};
		
		void informNodeGeneration(Cost, Cost) {}
		

		flt_t computeHBF_neighbours() {
			
			if(mLevelCount.empty() || mLevelCount.size() == 1)
				return 0;

			flt_t acc = 0;
			unsigned samples = 0;
			
			auto it = mLevelCount.begin();
			unsigned prevFlvl = it->first, prevCount = it->second;
			++it;
			
			for(; it != mLevelCount.end(); ++it) {
				unsigned flvl = it->first, count = it->second;
				
				flt_t r = (flt_t)count / prevCount;
				flt_t fDifRecip = 1.0 / (flvl - prevFlvl);
			
				flt_t bf = std::pow(r, fDifRecip);
				acc += bf;
				samples++;
				
				prevFlvl = flvl;
				prevCount = count;
			}
			
			return acc / samples;
		}
		
		
		
		flt_t computeHBF_refInit() {
			
			if(!mInitFvalSet || mLevelCount.empty() || mLevelCount.size() == 1)
				return 0;
			
			flt_t acc = 0;
			unsigned samples = 0;
			
			auto it = mLevelCount.begin();
			++it;
			
			unsigned initCount = mLevelCount[mInitNodeFval];
			
			for(; it != mLevelCount.end(); ++it) {
				unsigned flvl = it->first, count = it->second;
				
				if(count == initCount) {
					acc += 1;
					samples++;
					continue;
				}
				
				flt_t r = (flt_t)count / initCount;
				flt_t fDifRecip = 1.0 / (flvl - mInitNodeFval);
			
				flt_t bf = std::pow(r, fDifRecip);
				acc += bf;
				samples++;
			}
			
			return acc / samples;
		}
		
		flt_t computeHBF() {
			if(mConf.use_hbf_ref_init)
				return computeHBF_refInit();
			else
				return computeHBF_neighbours();			
		}
		
		
		void reset() {
			mLevelCount.clear();
			mInitFvalSet = false;
		}
		
		
		HeuristicBF(AlgoConf<> const& pConf) :
			mConf(pConf)
		{
			reset();
		}

		std::map<Cost, unsigned> mLevelCount;
		Cost mInitNodeFval;
		bool mInitFvalSet;
		AlgoConf<> const& mConf;
	};
	
	
	


	
	template<typename = void>
	struct UGSABehaviour : public HeuristicBF<ucost_t> {
		
		using Cost = ucost_t;
		
		UGSABehaviour(AlgoConf<> const& pConf) :
			HeuristicBF<ucost_t>(pConf),
			mPref(pConf.kpref),
			mConf(pConf)
		{
			reset();
		}
		
		ucost_t compute_singleTree(unsigned pLvl, ucost_t pG) {
			slow_assert(pLvl == 1);
			
			flt_t remExpFlt = std::pow(mCachedHBF, pG); //<- dist correction?
			
			ucost_t remExp;
			
			if(remExpFlt < 100000000)
				remExp = remExpFlt;
			else {
				remExp = 100000000;
				mClipCount++;
			}
			
			slow_assert((flt_t)remExp * mPref < std::numeric_limits<ucost_t>::max() / 2, "%f %f %d", mCachedHBF, remExp, pG );
			
			return pG + mPref * remExp;
		}

		
		unsigned compute_effectiveEdge(unsigned pLvl, Cost pG) {
			return compute_singleTree(pLvl, pG) * (mConf.useAllFrontier ? mBaseFrontierSz : 1);
		}
		
		void informAbtSearchBegins(unsigned pLvl, unsigned pFrontierSz) {
			mCachedHBF = this->computeHBF();
			mBaseFrontierSz = pFrontierSz;
			slow_assert(!mIsInAbtSearch);
			mIsInAbtSearch = true;
		}
		
		void informAbtSearchEnds() {
			slow_assert(mIsInAbtSearch);
			mIsInAbtSearch = false;
		}
		
		void reset() {
			HeuristicBF<ucost_t>::reset();
			mClipCount = 0;
			mIsInAbtSearch = false;
		}
		
		Json report() {
			Json j;
			j["hbf"] = this->computeHBF();
			j["used k pref"] = mPref;
			j["used hbf ref init"] = mConf.use_hbf_ref_init;
			j["used all frontier"] = mConf.useAllFrontier;
			j["clips"] = mClipCount;
			return j;
		}
		
		
		const unsigned mPref;
		AlgoConf<> const& mConf;
		unsigned mClipCount;
		ucost_t mCachedHBF;
		unsigned mBaseFrontierSz;
		bool mIsInAbtSearch;
	};
}}}
