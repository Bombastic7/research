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
	
/*
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
				
				unsigned expCount = mCountMap[d].val;
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
			unsigned refExpCount = mCountMap[mRefLvl].val;
			
			slow_assert(refExpCount != 0);
			
			flt_t avgBFacc = 0;
			
			for(unsigned l : lvlsList) {
				if(l == mRefLvl) {
					avgBFacc += 1;
					continue;
				}

				avgBFacc += std::pow((flt_t)mCountMap[l].val/refExpCount, 1.0/(l - refExpCount));
			}
			
			return avgBFacc / mCountMap.size();
		}
		
		HeuristicBF() :
			mRefLvlSet(false)
		{}

		private:
		
		SimpleHashMap<unsigned, unsigned, 1000> mCountMap;
		unsigned mRefLvl;
		bool mRefLvlSet;
	};
	
	
*/


	
	template<typename = void>
	struct UGSABehaviour {
		
		const flt_t c_wf, c_wt;
		const unsigned c_binWidth;
		
				
		UGSABehaviour(Json const& jConfig) :
			c_wf(jConfig.at("wf")),
			c_wt(jConfig.at("wt")),
			c_binWidth(jConfig.at("bin_width")),
			mBaseExpd(),
			mHasStarted(),
			mNextRecalc(),
			mCachedBF()
		{
			fast_assert(c_wf >= 0 && c_wt >= 0);
			fast_assert(c_binWidth > 0);
			
			reset();
		}
		

		void start() {
			fast_assert(!mHasStarted);
			mHasStarted = true;
			mExpTime.start();
		}
		
		double getBranchingFactor() {
			return mCachedBF;
		}
		
		double getExpansionTime() {
			return mExpTime.getExpansionTime();
		}
		
		void informNodeExpansion(ucost_t pUval) {
			slow_assert(mHasStarted);
			mBaseExpd++;
			
			ucost_t norm_u = pUval + mBaseExpd * mExpTime.getExpansionTime();
			
			mExpTime.informNodeExpansion();
			mUHist[assignBin(norm_u)].val++;			
			
			if(mBaseExpd >= mNextRecalc) {
				mNextRecalc *= 2;
				
				mCachedBF = compute_bf();
				mExpTime.update();
			}
		}
		
		void reset() {
			mUHist.clear();
			mExpTime.reset();
			mBaseExpd = 0;
			mHasStarted = false;
			mNextRecalc = 16;
			mCachedBF = 1;

		}
		

		Json report() {
			Json j;
			j["bf"] = mCachedBF;
			j["exptime"] = mExpTime.report();
			j["used wf"] = c_wf;
			j["used wt"] = c_wt;
			j["used bin width"] = c_binWidth;
			return j;
		}
		
		
		
		private:
		
		double compute_bf() {
			std::vector<unsigned> bins = mUHist.orderedKeys();
			
			double acc = 0;
			
			for(unsigned i=1; i<bins.size(); i++) {
				acc += std::pow((double)mUHist[bins[i]].val/mUHist[bins[0]].val, 1.0/((double)bins[i] - bins[0]));
			}
			
			return acc / (bins.size()-1);
		}
		
		
		
		unsigned assignBin(ucost_t pUval) {
			slow_assert(std::numeric_limits<unsigned>::max() > pUval);
			
			return ((unsigned)pUval / c_binWidth) * c_binWidth;
		}
		



		ComputeExpansionTime<> mExpTime;
		SimpleHashMap<unsigned, unsigned, 10000> mUHist;
		

		unsigned mBaseExpd;
		bool mHasStarted;
		unsigned mNextRecalc;
		
		double mCachedBF;

	};
}}}
