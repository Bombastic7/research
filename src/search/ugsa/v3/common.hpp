#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/time.hpp"	
	

namespace mjon661 { namespace algorithm { namespace ugsav3 {	

	using Util_t = double;
	using flt_t = double;


	template<typename Cost>
	struct AbtSearchResult {
		Util_t ug;
		Cost g;
		unsigned depth;
	};




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
	struct DepthCompression {
		
		void informPathDif(int pBdist, int pAdist) {
			if(pAdist == 0)
				return;

			mAbtCompAcc += pBdist / pAdist;
			mNsamples++;
		}
		
		double getDepthCompression() {
			return mathutil::max(1.0, mAbtCompAcc / mNsamples);
		}
		
		void reset() {
			mAbtCompAcc = 1;
			mNsamples = 1;
		}
		
		private:
		flt_t mAbtCompAcc;
		unsigned mNsamples;
	};
	 
	template<typename = void>
	struct CostCompression {
		
		void informCostDif(flt_t pBcost, flt_t pAcost) {
			if(pAcost == 0)
				return;
			
			mAbtCompAcc += pBcost / pAcost;
			
			
			if((mNsamples-1) % 100 == 0 || mNsamples < 100) {//.............
				std::cout << mNsamples << ": " << mAbtCompAcc/mNsamples << ", " << pBcost<< ", " << pAcost << "\n";
			}
			mNsamples++;
		}
		
		double getCostCompression() {
			return mathutil::max(1.0, mAbtCompAcc / mNsamples);
		}
		
		void reset() {
			mAbtCompAcc = 1;
			mNsamples = 1;
		}
		
		private:
		flt_t mAbtCompAcc;
		unsigned mNsamples;
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
	
	
	template<typename = void>
	struct UGSABehaviour : public CostCompression<>, public DepthCompression<>, public ComputeAvgBF<> {

		UGSABehaviour(Util_t pwf, Util_t pwt) :
			wf(pwf), wt(pwt)
		{
			reset();
		}
		
		Util_t compute_ug(unsigned pLvl, flt_t pG, unsigned pDistance) {
			
			return 	wf * this->getCostCompression() * pG + 
					wt * pow(this->getAvgBF(), this->getDepthCompression() * pDistance);
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
		
		
		void reset() {
			CostCompression<>::reset();
			DepthCompression<>::reset();
			ComputeAvgBF<>::reset();
			mLastExpd = 0;
			mNextExpd = 16;
		}
		
		Json report() {
			Json j;
			j["avg bf"] = this->getAvgBF();
			j["depth comp"] = this->getDepthCompression();
			j["cost comp"] = this->getCostCompression();
			return j;
			
		}
		
		const flt_t wf, wt;
		unsigned mLastExpd, mNextExpd;
	};	
	
	
}}}
