#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/math.hpp"
#include "search/solution.hpp"

#include "search/hastar/v2/common.hpp"
#include "search/hastar/v2/base_search.hpp"
//#include "search/hastar/v2/level_stats.hpp"
#include "search/hastar/v2/simple_stats.hpp"


namespace mjon661 { namespace algorithm { namespace hastarv2 {
	

	
	template<typename DomStack, typename StatsManager, unsigned Top_Limit>
	struct HAstar {
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		
		static const unsigned Top_Abt_Level_Used = mathutil::min(DomStack::Top_Abstract_Level, Top_Limit);
		
		HAstar(DomStack& pStack, Json const& jConfig) :
			mStatsManager(),
			mAlgo(pStack, jConfig, mStatsManager)
		{
			
		}
		
		
		void execute(Solution<BaseDomain>& pSol) {
			mAlgo.doSearch(pSol);
		}

		Json report() {
			mAlgo.submitStats();
			Json j = mStatsManager.report();
			return j;
		}
		
		void reset() {
			mAlgo.reset();
			mStatsManager.reset();
		}


		StatsManager mStatsManager;
		HAstar_Base<DomStack, Top_Abt_Level_Used, StatsManager> mAlgo;
		
	};
	
	/*
	
	template<typename D>
	using HAstar_StatsLevel = HAstar<D, LevelStatsManager<>, 1000>;
	
	template<typename D>
	using HAstar_StatsLevel_1lvl = HAstar<D, LevelStatsManager<>, 1>;
	*/
	template<typename D>
	using HAstar_StatsSimple = HAstar<D, SimpleStatsManager<>, 1000>;
	
	template<typename D>
	using HAstar_StatsSimple_1lvl = HAstar<D, SimpleStatsManager<>, 1>;
	
	
	
	
	
	
	
	template<typename DomStack, typename StatsManager, bool Use_Depth, unsigned Top_Limit>
	struct PDB_HAstar {
		
		static const unsigned Top_Abt_Level_Used = mathutil::min(DomStack::Top_Abstract_Level, Top_Limit);
		
		using BaseDomain = typename DomStack::template Domain<0>;
		using State = typename BaseDomain::State;
		using Cost = CostDepthImpl<Use_Depth, typename Domain::Cost>::type;
		
		using Algo_t = HAstar_Abt<DomStack, 1, Top_Abt_Level_Used+1, Use_Depth, StatsManager>;
		
		
		
		PDB_HAstar(DomStack& pStack) :
			mStatsManager(),
			mAlgo(pStack, {{"do_caching":true}}, mStatsManager)
		{}
		
		Cost eval(State const& pState) {
			return mAlgo.doSearch(pState);
		}

		Json report() {
			mAlgo.submitStats();
			Json j = mStatsManager.report();
			return j;
		}
		
		void reset() {
			mAlgo.reset();
			mAlgo.clearCache();
			mStatsManager.reset();
		}

		StatsManager mStatsManager;
		Algo_t mAlgo;
	};


	template<typename DomStack, bool Use_Depth>
	using PDB_HAstar_SimpleStats<DomStack, SimpleStatsManager, Use_Depth, 10000>;
}}}
