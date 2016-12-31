#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"



namespace mjon661 { namespace algorithm { namespace hastarv2 {
	

	/*
	 * Full stats for base level, per-search and per-level for abstract levels.
	 */
	template<typename = void>
	struct SimpleStatsManager {
		
		template<unsigned L>
		struct StatsAcc {
			void a_expd() {mExpd++;}
			void a_gend() { mGend++; }
			void a_dups() { if(L==0) mDups++; }
			void a_reopnd() { if(L==0) mReopnd++; }
			
			void l_cacheMadeExact() {}
			void l_cacheImprove() { }
			void l_cacheAdd() {  }
			
			void s_openListSize(unsigned sz) {  }
			void s_closedListSize(unsigned sz) { }
			void s_solutionPartial() {  }
			void s_solutionFull() { ; }
			void s_cachePartial() { }
			void s_cacheMiss() {  }
			void s_cacheHit() {  }
			
			void s_end() { 				
				mNsearches++;
				mTotalExpd += mExpd;
				mTotalGend += mGend;
				searchCountsReset();
			}

			
			unsigned mExpd, mGend, mDups, mReopnd;
			unsigned mNsearches, mTotalExpd, mTotalGend;
			
			
			StatsAcc(SimpleStatsManager<>& pManager) :
				mManager(pManager)
			{
				reset();
			}
			
			void searchCountsReset() {
				mExpd = mGend = mDups = mReopnd = 0;
			}
			
			void reset() {
				searchCountsReset();
				mNsearches = 0;
				mTotalExpd = mTotalGend = mNsearches = 0;
	
			}

			
			void submit() {
				Json j;
				
				if(L == 0) {
					mManager.mReport["_base_expd"] = mExpd;
					mManager.mReport["_base_gend"] = mGend;
					j["dups"] = mDups;
					j["reopnd"] = mReopnd;
				}

				j["NSearches"] = mNsearches;
				j["expd"] = mExpd;
				j["gend"] = mGend;


				mManager.mReport[std::string("Level ") + std::to_string(L)] = j;
				mManager.mAllExpd += mTotalExpd;
				mManager.mAllGend += mTotalGend;
			}
			
			private:
			SimpleStatsManager<>& mManager;

		};

		
		
		SimpleStatsManager() {
			reset();
		}
		
		Json report() {
			mReport["_all_expd"] = mAllExpd;
			mReport["_all_gend"] = mAllGend;
			return mReport;
		}
		
		
		void reset() {
			mAllExpd = mAllGend = 0;
			mReport = Json();
		}
		
		unsigned mAllExpd, mAllGend;
		Json mReport;
	};
}}}
