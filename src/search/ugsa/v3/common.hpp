#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/time.hpp"	
	

namespace mjon661 { namespace algorithm { namespace ugsav3 {	

	using Util_t = float;
	using flt_t = float;




	template<typename Domain, typename CacheEntry>
	struct CacheStore {

		using PackedState = typename Domain::PackedState;
		
		
		struct StoreOps {
			StoreOps(Domain const& pDomain) :
				mDomain(pDomain)
			{}
			
			size_t operator()(PackedState const& pkd) const {
				return mDomain.hash(pkd);
			}
			
			size_t operator()(CacheEntry * const ent) const {
				return mDomain.hash(ent->pkd);
			}
			
			bool operator()(CacheEntry * const ent, PackedState const& pkd) const {
				return mDomain.compare(ent->pkd, pkd);
			}
			
			Domain const& mDomain;
		};
		
		using Store_t = ClosedList<CacheEntry, 
									CacheEntry,
									PackedState, 
									StoreOps,
									StoreOps,
									Domain::Hash_Range>;

		using Pool_t = NodePool<CacheEntry, typename Store_t::Wrapped_t>;
		
		using iterator = typename Store_t::iterator;
		
		
		CacheStore(Domain const& pDomain) :
			mStore(StoreOps(pDomain), StoreOps(pDomain)),
			mPool()
		{}
		
		CacheEntry* retrieve(PackedState const& pkd) {
			CacheEntry* ent = mStore.find(pkd);
			return ent;
		}
		
		bool get(PackedState const& pkd, CacheEntry*& ent) {
			bool retNew = false;
			
			ent = mStore.find(pkd);
			
			if(!ent) {
				ent = mPool.construct();
				ent->pkd = pkd;
				mStore.add(ent);
				retNew = true;
			}
			return retNew;
		}
		
		void clear() {
			mStore.clear();
			mPool.clear();
		}
		
		iterator begin() {
			return mStore.begin();
		}
		
		iterator end() {
			return mStore.end();
		}

		
		private:
		Store_t mStore;
		Pool_t mPool;
	};
	



	template<typename = void>
	struct ComputeExpansionTime {
		void informNodeExpansion() {
			if(!mRunning) {
				mTimer.start();
				mRunning = true;
			}
			
			mExpThisPhase++;
			
			if(mExpThisPhase >= mNextCalc) {
				mTimer.stop();
				mTimer.start();
				mExpTime = mTimer.seconds() / mExpThisPhase;
				mNextCalc *= 2;
				mExpThisPhase = 0;
				mNphases++;
			}
		}
		
		flt_t getExpansionTime() {
			return mExpTime;
		}
		
		void reset() {
			mExpThisPhase = mNphases = 0;
			mExpTime = 0;
			mNextCalc = 16;
			mTimer.stop();
			mRunning = false;
		}
		
		Json report() {
			Json j;
			j["phase count"] = mNphases;
			j["expd this phase"] = mExpThisPhase;
			j["exp time"] = mExpTime;
			j["next calc"] = mNextCalc;
			return j;
		}
		
		ComputeExpansionTime() {
			//reset();
		}
		
		
		private:
		unsigned mExpThisPhase, mNextCalc, mNphases;
		flt_t mExpTime;
		Timer mTimer;
		bool mRunning;
	};


	
	template<typename = void>
	struct ComputeAvgBF {
		
		ComputeAvgBF() :
			mDepthCount(100),
			mDepthBF(100)
		{
			//reset();
		}
		
		void informNodeExpansion(unsigned pDepth) {
			if(pDepth == 0)
				return;
			
			if(pDepth >= mDepthCount.size()) {
				mDepthCount.resize(mDepthCount.size()*2, 0);
				mDepthBF.resize(mDepthCount.size()*2, 0);
			}
			
			if(pDepth > mTopDepth) {
				slow_assert(pDepth == mTopDepth + 1);
				mTopDepth++;
			}

			mAvgBFAcc -= mDepthBF[pDepth];
			mDepthBF[pDepth] = std::pow(++mDepthCount[pDepth], 1.0/pDepth);
			mAvgBFAcc += mDepthBF[pDepth];
			
			mTotalExpansions++;
		}
		
		double getAvgBF() {
			return mAvgBFAcc / mTopDepth;
		}
		
		void reset() {
			std::fill(mDepthCount.begin(), mDepthCount.end(), 0);
			std::fill(mDepthBF.begin(), mDepthBF.end(), 0);
			mDepthCount[0] = 1;
			mAvgBFAcc = 1;
			mTopDepth = 1;
			mTotalExpansions = 0;
		}
		
		unsigned getTotalExpansions() {
			return mTotalExpansions;
		}

		private:
		std::vector<unsigned> mDepthCount;
		std::vector<flt_t> mDepthBF;
		
		flt_t mAvgBFAcc;
		unsigned mTopDepth;
		unsigned mTotalExpansions;
	};
	
	
	template<unsigned Bound>
	struct AbtEdgeCorrection {
		
		void informPath(unsigned pLvl, flt_t pCost, flt_t pDist) {			
			if(pLvl == 0) {
				if(pCost == 0)
					return;
				mPathRatiosAcc[0] += pDist / pCost;
			}
			else {
				if(pDist == 0)
					return;
				mPathRatiosAcc[pLvl] += pCost / pDist;
			}
			
			mSampleCount[pLvl] += 1;
		}
		
		flt_t getDistCorrection(unsigned pLvl) {
			slow_assert(pLvl > 0);
			flt_t f = (mPathRatiosAcc[0] / mSampleCount[0]) * (mPathRatiosAcc[pLvl] / mSampleCount[pLvl]);
			slow_assert(std::isfinite(f) && f > 0);
			return f;
		}
		
		void reset() {
			mPathRatiosAcc.fill(1);
			mSampleCount.fill(1);
		}
		
		Json report() {
			Json jReport, jCostDist, jCorrection, jSamples;
			
			for(unsigned i=0; i<Bound; i++) {
				flt_t r = mPathRatiosAcc[0] / mSampleCount[0];
				
				if(i == 0)
					r = 1.0/r;
				
				std::string key = std::to_string(i);
				
				jCostDist[key] = r;
				jCorrection[key] = getDistCorrection(i);
				jSamples[key] = mSampleCount[i];
			}
			
			jReport["cost/dist"] = jCostDist;
			jReport["dist correction"] = jCorrection;
			jReport["samples"] = jSamples;
			
			return jReport;
		}
		
		AbtEdgeCorrection() {
			//reset();
		}

		private:
		std::array<flt_t, Bound> mPathRatiosAcc;
		std::array<flt_t, Bound> mSampleCount;	
	};
	
	
	template<unsigned Bound>
	struct UGSABehaviour : public AbtEdgeCorrection<Bound>, public ComputeAvgBF<>, public ComputeExpansionTime<> {

		UGSABehaviour(Util_t pwf, Util_t pwt) :
			wf(pwf), wt(pwt)
		{
			reset();
		}
		
		Util_t compute_ug(unsigned pLvl, flt_t pG, unsigned pDistance) {
			
			return 	wf * pG + 
					wt * pow(this->getAvgBF(), this->getDistCorrection(pLvl) * pDistance) * this->getExpansionTime();
		}
		
		bool shouldUpdate() {
			unsigned exp = this->getTotalExpansions();
			
			if(exp <= 10)
				return true;
				
			if(exp >= mNextExpd) {
				mNextExpd *= 2;
				return true;
			}
			return false;
		}
		
		void informNodeExpansion(unsigned pDepth) {
			ComputeAvgBF<>::informNodeExpansion(pDepth);
			ComputeExpansionTime<>::informNodeExpansion();
		}
		
		
		void reset() {
			AbtEdgeCorrection<Bound>::reset();
			ComputeAvgBF<>::reset();
			ComputeExpansionTime<>::reset();
			mLastExpd = 0;
			mNextExpd = 16;
		}
		
		Json report() {
			Json j = AbtEdgeCorrection<Bound>::report();
			j["avg bf"] = this->getAvgBF();
			j["exp time"] = ComputeExpansionTime<>::report();
			return j;
			
		}
		
		const flt_t wf, wt;
		unsigned mLastExpd, mNextExpd;
	};	
	
	
}}}
