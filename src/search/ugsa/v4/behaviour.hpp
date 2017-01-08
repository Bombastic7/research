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
#include "structs/simple_hashmap.hpp"

#include "search/ugsa/v4/defs.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {	


	template<typename = void>
	struct ComputeExpansionTime {
		void informNodeExpansion() {
			mExpThisPhase++;
		}
		
		flt_t getExpansionTime() {
			return mExpTime;
		}
		
		Json report() {
			Json j;
			j["phase count"] = mNphases;
			j["expd this phase"] = mExpThisPhase;
			j["exp time"] = mExpTime;
			j["next calc"] = mNextCalc;
			return j;
		}
		
		void update() {
			mTimer.stop();
			mTimer.start();
			mExpTime = mTimer.seconds() / mExpThisPhase;
			mExpThisPhase = 0;
			mNphases++;
		}
		
		void start() {
			mTimer.start();
		}
		
		void reset() {
			mExpThisPhase = mNphases = 0;
			mExpTime = 0;
			mTimer.stop();
		}
		
		private:
		unsigned mExpThisPhase, mNphases;
		flt_t mExpTime;
		Timer mTimer;
	};
	

	template<typename = void>
	struct ComputeAvgBF {
		
		void informNodeExpansion(unsigned pDepth) {
			mCountMap[pDepth].val++;
		}
		
		flt_t computeAvgBF() {
			std::vector<unsigned> const& depthsList = mCountMap.unorderedKeys();
			
			flt_t avgBFacc = 0;
			
			for(unsigned d : depthsList) {
				if(d == 0) {
					avgBFacc += 1;
					continue;
				}
				
				expCount = mCountMap[d].val;
				avgBFacc += std::pow(expCount, 1.0/d);
			}
			
			return avgBFacc / mCountMap.size();
		}
		
		void reset() {
			mCountMap.clear();
		}

		private:
		
		SimpleHashMap<unsigned, unsigned, 1000> mCountMap;
	};




	template<typename = void>
	struct HeuristicBF {

		void informNodeExpansion(unsigned pLvl) {
			mCountMap[pLvl].val++;	
		}
		
		void setRefLvl(unsigned pLvl) {
			mRefLvl = pLvl;
			mRefLvlSet = true;
		}
		
		void reset() {
			mCountMap.clear();
			mRefLvlSet = false;
		}
		
		flt_t computeAvgHBF_ref() {
			slow_assert(mRefLvlSet);
			
			std::vector<unsigned> const& lvlsList = mCountMap.unorderedKeys();
			unsigned refExpCount = mCountMap[mRefLvl];
			
			slow_assert(refExpCount != 0);
			
			flt_t avgBFacc = 0;
			
			for(unsigned l : lvlsList) {
				if(l == mRefLvl) {
					avgBFacc += 1;
					continue;
				}

				avgBFacc += std::pow(expCount, 1.0/(l - refExpCount));
			}
			
			return avgBFacc / mCountMap.size();
		}
		
		HeuristicBF() :
			mInitValSet(false)
		{}

		private:
		
		SimpleHashMap<unsigned, unsigned, 1000> mCountMap;
		unsigned mRefLvl;
		bool mRefLvlSet;
	};
	
	
	/*
	 * 	Provides:
	 * 		compute_U(g, depth): Translates abstract level 1 g/depth to U for base level, and to guide abt search.
	 * 
	 * 		informNodeExpansion(g, u): Notification that a base space expansion occured.
	 * 
	 * 		abtShouldCache(): True if abt level 1 should perform u*-caching.
	 * 
	 * 		abtHtoU(Cost h): Translates a cost heuristic in abt lvl 2+ into abt 1 U value.
	 * 
	 * 	Params:
	 * 		bool use_hbf_init: True: Compute hbf as average of count(lvl) vs count(init lvl).
	 * 							False: as average of all pairs of neighbouring counts. e.g. count(lvl_i) vs count(lvl_j), j vs k..
	 * 
	 * 		bool g_for_hbf:		Calculate hbf with g-values rather than f.
	 * 
	 * 		
	 */


	
	template<typename Domain>
	struct UGSABehaviour {
		
		using Cost = typename Domain::Cost;
		
		enum { Use_HBF, Use_Avg_BF };
		
		
		
		UGSABehaviour(Json const& jConfig) :
			c_treeSizeMethod(parseOption(jConfig, "tree size method")),
			c_wf(jConfig.at("wf")),
			c_wt(jConfig.at("wt"))
		{
			fast_assert(c_wf >= 0 && c_wt >= 0);
			
			reset();
			hasStarted = false;
		}
		
		
		
		
		
		
		void start() {
			fast_assert(!hasStarted);
			mHasStarted = true;
			mExpTime.start();
		}
		
		unsigned compute_U(Cost pgval, unsigned pDepth) {
			slow_assert(mHasStarted);
			return compute_singleTree(pgval, pDepth);
		}
		
		void informNodeExpansion(Cost pgval, Cost pfval, ucost_t puval, unsigned pDepth) {
			if(c_treeSizeMethod == Use_HBF)
				mHBF.informNodeExpansion(pgval);
			
			else
				mAvgBF.informNodeExpansion(pDepth);
			
			mExpTime.informNodeExpansion();
		}
		
		bool abtShouldCache() {
			return this->totalBaseExpansion() > 100;
		}
		
		ucost_t abtHtoU(Cost pCost) {
			return mConf.wf * pCost;
		}
		
		ucost_t abtDtoU(unsigned pDepth) {
			return mConf.wt * std::pow(this->computeHBF(), pDepth);
		}
		
		void reset() {
			mHBF.reset();
			mAvgBF.reset();
			mExpTime.reset();
			mClipCount = 0;
			mBaseExpd = 0;
			mHasStarted = false;
		}
		
		Json report() {
			Json j;
			j["hbf"] = this->computeHBF();
			j["exptime"] = ComputeExpansionTime<>::report();
			j["used wf"] = c_wt;
			j["used wt"] = c_wt;
			j["clips"] = mClipCount;
			return j;
		}
		
		
		
		int parseOption(Json const& jConfig, std::string const& key) {
			std::string val = jConfig.at(key);
			
			if(key == "tree size method") {
				if(val == "Use_HBF") return Use_HBF;
				else if(val == "Use_Avg_BF") return Use_Avg_BF;
				else gen_assert(false);
			}
			
			gen_assert(false);
			return 0;
		}
		
		
		ucost_t compute_singleTree(Cost pgval, ucost_t pDepth) {
			
			flt_t remExpFlt = std::pow(this->computeHBF(), pgval); //<- dist correction?

			if(remExpFlt > 100000000) {
				remExpFlt = 100000000;
				mClipCount++;
			}

			return c_wf * pgval + c_wt * remExpFlt * this->getExpansionTime();
		}
		
		
		
		
		

		
		HeuristicBF<> mHBF;
		ComputeAvgBF<> mAvgBF;
		ComputeExpansionTime<> mExpTime;

		
		unsigned mClipCount;
		unsigned mBaseExpd;
		bool mHasStarted;
		//ucost_t mCachedHBF;
		//unsigned mBaseFrontierSz;
		
		const int c_treeSizeMethod;
		const flt_t c_wf, c_wt;
		
		
	};
}}}
