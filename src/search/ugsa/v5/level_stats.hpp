#pragma once

#include <string>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/sum.hpp>

#include "util/debug.hpp"
#include "util/json.hpp"



namespace mjon661 { namespace algorithm { namespace hastarv2 {
	
	namespace ba = boost::accumulators; 
	
	/*
	 * Full stats for base level, per-search and per-level for abstract levels.
	 */
	template<typename = void>
	struct LevelStatsManager {
		
		template<unsigned L, typename = void>
		struct StatsAcc {
			void a_expd() {mExpd++; if(L==0) mL0expd++; }
			void a_gend() { if(L==0) mL0gend++; }
			void a_dups() { if(L==0) mL0dups++; }
			void a_reopnd() { if(L==0) mL0reopnd++; }
			
			void l_cacheMadeExact() { mCacheMadeExact++; }
			void l_cacheImprove() { mCacheImprove++; }
			void l_cacheAdd() { mCacheAdd++; }
			
			void s_openListSize(unsigned sz) { mAccOpen(sz); }
			void s_closedListSize(unsigned sz) { mAccClosed(sz); }
			void s_solutionPartial() { mSolPartial++; }
			void s_solutionFull() { mSolFull++; }
			void s_cachePartial() { mCachePartial++; }
			void s_cacheMiss() { mCacheMiss++; }
			void s_cacheHit() { mCacheHit++; }
			
			void s_end() { 
				mAccExpd(mExpd);
				mAccCacheImprove(mCacheImprove);
				mAccCacheAdd(mCacheAdd);
				mAccCacheMadeExact(mCacheMadeExact);
				
				mNsearches++;
				searchCountsReset();
			}
			
			using Acc_t = ba::accumulator_set<double, ba::features<ba::tag::min, ba::tag::max, ba::tag::mean, ba::tag::sum>>;
			
			Acc_t mAccExpd, mAccOpen, mAccClosed, mAccCacheImprove, mAccCacheAdd, mAccCacheMadeExact;
			
			unsigned mExpd, mCacheMadeExact, mCacheAdd, mCacheImprove;
			
			
			unsigned mCacheHit, mCacheMiss, mCachePartial;
			unsigned mSolPartial, mSolFull;
			unsigned mNsearches;
			
			unsigned mL0expd, mL0gend, mL0dups, mL0reopnd;
			
			
			StatsAcc(LevelStatsManager<>& pManager) :
				mManager(pManager)
			{
				reset();
			}
			
			void searchCountsReset() {
				mExpd = mCacheMadeExact = mCacheAdd = mCacheImprove = 0;
			}
			
			void reset() {
				searchCountsReset();
				mCacheHit = mCacheMiss = mCachePartial = 0;
				mSolPartial = mSolFull = mNsearches = 0;
				mExpd = mNsearches = 0;
				
				mL0expd = mL0gend = mL0dups = mL0reopnd = 0;
				
				mAccExpd = Acc_t();
				mAccOpen = Acc_t();
				mAccClosed = Acc_t();
				mAccCacheImprove = Acc_t();
				mAccCacheAdd = Acc_t();
				mAccCacheMadeExact = Acc_t();
			}
			
			std::string accPrettyStr(Acc_t const& acc) {
				return 	std::to_string(ba::min(acc)) + " " + 
						std::to_string(ba::max(acc)) + " " + 
						std::to_string(ba::mean(acc)) + " " +
						std::to_string(ba::sum(acc));
			}
			
			void submit() {
				Json j;
				
				if(L == 0) {
					j["expd"] = mL0expd;
					j["gend"] = mL0gend;
					j["dups"] = mL0dups;
					j["reopnd"] = mL0reopnd;
					mManager.mReport["_base_expd"] = mL0expd;
					mManager.mReport["_base_gend"] = mL0gend;
					
				}
				else {
					j["expd"] = accPrettyStr(mAccExpd);
					j["open sz"] = accPrettyStr(mAccOpen);
					j["closed sz"] = accPrettyStr(mAccClosed);
					j["Cache improved"] = accPrettyStr(mAccCacheImprove);
					j["Cache added"] = accPrettyStr(mAccCacheAdd);
					j["Cache made exact"] = accPrettyStr(mAccCacheMadeExact);
					j["NSearches"] = mNsearches;
				}
				
				mManager.mReport[std::string("Level ") + std::to_string(L)] = j;
				mManager.mAllExpd += ba::sum(mAccExpd);
			}
			
			private:
			LevelStatsManager<>& mManager;

		};

		
		
		LevelStatsManager()
		{
			reset();
		}
		
		Json report() {
			mReport["_all_expd"] = mAllExpd;
			mReport["_all_gend"] = 0;
			return mReport;
		}
		
		
		void reset() {
			mAllExpd = 0;
			mReport = Json();
		}
		
		unsigned mAllExpd;
		Json mReport;
	};
}}}
