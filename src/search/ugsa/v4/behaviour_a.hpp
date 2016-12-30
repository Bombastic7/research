#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/time.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {	

	namespace impl_behaviour_a {
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
			
			ComputeExpansionTime(unsigned pFirstUpdate = 16, unsigned pUpdateFactor = 4) :
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


		
		template<typename = void>
		struct ComputeAvgBF {
			
			struct DepthCountData {
				unsigned nInc, nUncached;
				flt_t bf_cached;
				
				DepthCountData() :
					nInc(0), nUncached(0), bf_cached(0)
				{}
			};
			
			static const unsigned Refresh_Factor = 10;
			
			ComputeAvgBF()
			{
				//reset();
			}
			
			void informNodeExpansion(unsigned pDepth, bool pForceRefresh = false) {
				if(pDepth == 0)
					return;

				
				mDirty = true;
				
				slow_assert(pDepth <= mDepthCount.size());
				
				if(pDepth == mDepthCount.size())
					mDepthCount.push_back(DepthCountData());
				
				DepthCountData& d = mDepthCount[pDepth];
				
				d.nUncached++;
				
				if(pForceRefresh || d.nUncached >= Refresh_Factor) {
					mAvgBFAcc -= d.bf_cached;
					d.nInc += d.nUncached;
					d.nUncached = 0;
					d.bf_cached = std::pow(d.nInc, 1.0/pDepth);
					mAvgBFAcc += d.bf_cached;
				}
			}
			
			flt_t getAvgBF() {
				if(mDirty) {
					mDirty = false;
					mAvgBF_cached = mAvgBFAcc / mDepthCount.size();
				}
				return mAvgBF_cached;
			}
			
			unsigned getTotalExpansions() {
				return mTotalExpansions;
			}
			
			void reset() {
				mDepthCount = std::vector<DepthCountData>(1);
				mDepthCount[0].nInc = 1;
				mDepthCount[0].bf_cached = 1;
				mAvgBFAcc = mAvgBF_cached = 1;
				mDirty = false;
				mTotalExpansions = 1;
			}


			private:
			std::vector<DepthCountData> mDepthCount;
			flt_t mAvgBFAcc, mAvgBF_cached;
			bool mDirty;
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
	}
	
	template<unsigned Bound>
	struct UGSABehaviour_A : public AbtEdgeCorrection<Bound>, public ComputeAvgBF<>, public ComputeExpansionTime<> {

		UGSABehaviour(Util_t pwf, Util_t pwt) :
			wf(pwf), wt(pwt)
		{
			reset();
		}
		
		Util_t compute_ug(unsigned pLvl, flt_t pG, unsigned pDistance) {
			
			return 	wf * pG + 
					wt * pow(this->getAvgBF(), this->getDistCorrection(pLvl) * pDistance) * this->getExpansionTime();
		}
		
		//Need to integrate frontier size, or something derived from it.
		
		bool shouldUpdate() {
			//return false;//.........
			unsigned exp = this->getTotalExpansions();
			
			if(exp <= 10)
				return true;
				
			if(exp >= mNextExpd) {
				mNextExpd *= 10;
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
