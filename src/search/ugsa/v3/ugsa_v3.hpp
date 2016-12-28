#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "search/solution.hpp"

#include "search/ugsa/v3/common.hpp"
#include "search/ugsa/v3/base_search.hpp"
#include "search/ugsa/v3/level_stats.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {
	
	/*
	template<typename = void>
	struct LevelStatsManager {
		
		template<unsigned L, typename = void>
		struct StatsAcc {
			void expd() {mExpd++;}
			void gend() {}
			void dups() {}
			void reopnd() {}
			
			void end() {			
				mNSearches++;
			
			}
				
			StatsAcc(LevelStatsManager<>& pManager) :
				mManager(pManager)
			{
				reset();
			}
			
			void reset() {
				mExpd = mNSearches = 0;
			}
			
			void submit() {
				Json j;
				j["total expd"] = mExpd;
				j["NSearches"] = mNSearches;
				mManager.mReport[std::string("Level ") + std::to_string(L)] = j;
				mManager.mTotExpd += mExpd;
			}
			
			private:
			LevelStatsManager<>& mManager;
			unsigned mExpd, mNSearches;
		};
		
		
		template<typename Ign>
		struct StatsAcc<0,Ign> {
			void expd() {mExpd++;}
			void gend() {mGend++;}
			void dups() {mDups++;}
			void reopnd() {mReopnd++;}
			
			unsigned getExpd() {
				return mExpd;
			}
			
			void end() {}
			
			void submit() {
				Json j;
				j["expd"] = mExpd;
				j["gend"] = mGend;
				j["dups"] = mDups;
				j["reopnd"] = mReopnd;
				mManager.mReport["Level 0"] = j;
				mManager.mTotExpd += mExpd;
			}
			

			StatsAcc(LevelStatsManager<>& pManager) :
				mManager(pManager)
			{
				reset();
			}
			
			void reset() {
				mExpd = mGend = mDups = mReopnd = 0;
			}
			
			LevelStatsManager& mManager;
			unsigned mExpd, mGend, mDups, mReopnd;
			
		};
		
		
		LevelStatsManager() :
			mTotExpd(),
			mReport()
		{}
		
		Json report() {
			mReport["expd"] = mTotExpd;
			return mReport;
		}
		
		
		void reset() {
			mTotExpd = 0;
			mReport = Json();
		}
		
		unsigned mTotExpd;
		Json mReport;
	};
	*/
	
	
	
	
	
	template<typename DomStack, typename StatsManager>
	struct UGSAv3 {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		
		UGSAv3(DomStack& pStack, Json const& jConfig) :
			mBehaviour(jConfig.at("wf"), jConfig.at("wt")),
			mStatsManager(),
			mAlgo(pStack, mBehaviour, mStatsManager)
		{
			
		}
		
		
		void execute(Solution<BaseDomain>& pSol) {
			mAlgo.doSearch(pSol);
		}

		Json report() {
			mAlgo.submitStats();
			Json j = mStatsManager.report();
			j["behaviour"] = mBehaviour.report();
			return j;
		}
		
		void reset() {
			mAlgo.reset();
			mStatsManager.reset();
			mBehaviour.reset();
		}


		UGSABehaviour<DomStack::Top_Abstract_Level+1> mBehaviour;
		StatsManager mStatsManager;
		UGSAv3_Base<DomStack, DomStack::Top_Abstract_Level, StatsManager> mAlgo;
		
	};
	
	
	template<typename D>
	using UGSAv3_StatsLevel = UGSAv3<D, LevelStatsManager<>>;
	
}}}
