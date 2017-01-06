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


	template<typename = void>
	struct ComputeExpansionTime {
		void informNodeExpansion() {
			mExpThisPhase++;
			if(mExpThisPhase >= mNextCalc)
				update();
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
			mNextCalc *= mUpdateFactor;
		}
		
		protected:
		
		void start() {
			mTimer.start();
		}
		
		void reset() {
			mExpThisPhase = mNphases = 0;
			mExpTime = 0;
			mNextCalc = mFirstUpdate;
			mTimer.stop();
		}
		
		ComputeExpansionTime(unsigned pFirstUpdate = 16, unsigned pUpdateFactor = 2) :
			mFirstUpdate(pFirstUpdate),
			mUpdateFactor(pUpdateFactor)
		{
			//reset();
		}
		
		
		private:
		unsigned mExpThisPhase, mNphases, mNextCalc;
		flt_t mExpTime;
		Timer mTimer;
		const unsigned mFirstUpdate, mUpdateFactor;
	};
	




	template<typename Cost>
	struct HeuristicBF {

		void informNodeExpansion(Cost pgval, Cost pfval, ucost_t puval) {
			
			//~ if(mConf.use_g_for_hbf)
				//~ pfval = pgval;
				
			mLevelCount[(unsigned)pfval]++;
			
			if(!mInitValSet) {
				mInitValSet = true;
				mInitVal = pfval;
			}
			
			mTotalCount++;
		};
		
		//~ void informNodeGeneration(Cost, Cost) {}
		

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
			
			if(!mInitValSet || mLevelCount.empty() || mLevelCount.size() == 1)
				return 0;
			
			flt_t acc = 0;
			unsigned samples = 0;
			
			auto it = mLevelCount.begin();
			++it;
			
			unsigned initCount = mLevelCount[mInitVal];
			
			for(; it != mLevelCount.end(); ++it) {
				unsigned flvl = it->first, count = it->second;
				
				if(count == initCount) {
					acc += 1;
					samples++;
					continue;
				}
				
				flt_t r = (flt_t)count / initCount;
				flt_t fDifRecip = 1.0 / (flvl - mInitVal);
			
				flt_t bf = std::pow(r, fDifRecip);
				acc += bf;
				samples++;
			}
			
			return acc / samples;
		}
		
		flt_t computeHBF() {
			if(mDirty) {
				mDirty = false;
				if(mConf.hbf_ref_init)
					mLastBF = computeHBF_refInit();
				else
					mLastBF = computeHBF_neighbours();
			}
			
			return mLastBF;
		}
		
		unsigned totalBaseExpansion() {
			return mTotalCount;
		}
		
		
		void reset() {
			mLevelCount.clear();
			mTotalCount = 0;
			mInitValSet = false;
			mDirty = true;
		}
		
		
		HeuristicBF(AlgoConf<> const& pConf) :
			mConf(pConf)
		{
			reset();
		}

		std::map<Cost, unsigned> mLevelCount;
		unsigned mTotalCount;
		ucost_t mInitVal;
		bool mInitValSet;
		flt_t mLastBF;
		bool mDirty;
		AlgoConf<> const& mConf;
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
	 * 		bool use_frontier_sz: if false --
	 * 
	 * 		
	 */


	
	template<typename Domain>
	struct UGSABehaviour : protected HeuristicBF<typename Domain::Cost>, protected ComputeExpansionTime<> {
		
		using Cost = typename Domain::Cost;
		
		UGSABehaviour(AlgoConf<> const& pConf) :
			HeuristicBF<Cost>(pConf),
			ComputeExpansionTime<>(),
			mConf(pConf)
		{
			reset();
		}
		
		ucost_t compute_singleTree(Cost pgval, ucost_t pDepth) {
			
			flt_t remExpFlt = std::pow(this->computeHBF(), pgval); //<- dist correction?

			if(remExpFlt > 100000000) {
				remExpFlt = 100000000;
				mClipCount++;
			}
			
			//slow_assert((flt_t)remExp * mPref < std::numeric_limits<ucost_t>::max() / 2, "%f %f %d", mCachedHBF, remExp, pG );
			
			return mConf.wf * pgval + mConf.wt * remExpFlt * this->getExpansionTime();
		}

		
		unsigned compute_U(Cost pgval, unsigned pDepth) {
			return compute_singleTree(pgval, pDepth);
		}
		
		void informNodeExpansion(Cost pgval, Cost pfval, ucost_t puval, unsigned pDepth) {
			HeuristicBF<Cost>::informNodeExpansion(pgval, pfval, puval);
			ComputeExpansionTime<>::informNodeExpansion();
		}
		
		bool abtShouldCache() {
			return this->totalBaseExpansion() > 100;
		}
		
		ucost_t abtHtoU(Cost pCost) {
			return mConf.wf * pCost;
		}
		
		void reset() {
			HeuristicBF<Cost>::reset();
			ComputeExpansionTime<>::reset();
			mClipCount = 0;
		}
		
		Json report() {
			Json j;
			j["hbf"] = this->computeHBF();
			j["exptime"] = ComputeExpansionTime<>::report();
			j["used wf"] = mConf.wf;
			j["used wt"] = mConf.wt;
			j["used hbf_ref_init"] = mConf.hbf_ref_init;
			j["clips"] = mClipCount;
			return j;
		}
		
		
		//const unsigned mPref;
		AlgoConf<> const& mConf;
		unsigned mClipCount;
		//ucost_t mCachedHBF;
		//unsigned mBaseFrontierSz;
	};
}}}
