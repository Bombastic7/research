#pragma once

#include <string>
#include "search/hastar/generic/base_search.hpp"
#include "search/solution.hpp"
#include "util/json.hpp"

namespace mjon661 { namespace algorithm { namespace hastargeneric {

	template<typename D, unsigned Top, E_StatsInfo P_Stats_Info>
	struct HAstar {
		
		using Domain = typename D::template Domain<0>;
		using HAstarAlg = HAstar_Base<D, Top, P_Stats_Info>;
		
		
		HAstar(D& pDomStack, Json const& j) :
			mDomain(pDomStack),
			mAlgo(pDomStack),
			mStatsPrefix()
		{
			if(P_Stats_Info == E_StatsInfo::Search)
				mStatsPrefix = j.at("stats file prefix");
		}
		
		void execute(Solution<Domain>& pSol) {
			typename Domain::State s0 = mDomain.createState();
			
			mAlgo.fillSolution(pSol, s0);
		}
		
		Json report() {
			if(P_Stats_Info == E_StatsInfo::Search)
				mAlgo.dumpSearchesRec(mStatsPrefix);
			
			Json rep = mAlgo.report();
			unsigned totExpd = rep.at("level 0").at("expd");
			
			for(unsigned i=1; i<=Top; i++)
				totExpd += rep.at(std::string("Level ") + std::to_string(i)).at("total expd").get<unsigned>();
			
			rep["expd"] = totExpd;
			
			return rep;
		}
		
		void reset(bool pClearCache = false) {
			mAlgo.reset(pClearCache);
		}
		
		private:
		const Domain mDomain;
		HAstarAlg mAlgo;
		std::string mStatsPrefix;
	};


	template<typename D> using HAstar_StatsNone = HAstar<D, D::Top_Abstract_Level, E_StatsInfo::None>;
	template<typename D> using HAstar_StatsLevel = HAstar<D, D::Top_Abstract_Level, E_StatsInfo::Level>;
	template<typename D> using HAstar_StatsSearch = HAstar<D, D::Top_Abstract_Level, E_StatsInfo::Search>;

}}}
