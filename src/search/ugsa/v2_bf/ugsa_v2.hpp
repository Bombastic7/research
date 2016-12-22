#pragma once

#include <string>
#include "search/ugsa/v2_bf/base_search.hpp"
#include "search/ugsa/v2_bf/common.hpp"
#include "search/solution.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav2_bf {
	
	template<typename D, unsigned Top, E_StatsInfo P_Stats_Info>
	struct UGSAv2 {
		
		using Domain = typename D::template Domain<0>;
		using UGSAAlg = UGSAv2_Base<D, Top, P_Stats_Info>;
		
		
		UGSAv2(D& pDomain, Json const& j) :
			mAlgo(pDomain, j),
			mDomain(pDomain)
		{
			if(P_Stats_Info == E_StatsInfo::Search)
				mStatsPrefix = j.at("stats file prefix");
		}
		
		void execute(Solution<Domain>& pSol) {
			auto s0 = mDomain.createState();
			mAlgo.fillSolution(pSol, s0);
		}
		
		void reset(bool pClearCache = false) {
			mAlgo.reset(pClearCache);
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
		
		private:
		UGSAv2_Base<D, Top, P_Stats_Info> mAlgo;
		Domain mDomain;
		std::string mStatsPrefix;
	};
	
	template<typename D> using UGSAv2_StatsNone = UGSAv2<D, D::Top_Abstract_Level, E_StatsInfo::None>;
	template<typename D> using UGSAv2_StatsLevel = UGSAv2<D, D::Top_Abstract_Level, E_StatsInfo::Level>;
	template<typename D> using UGSAv2_StatsSearch = UGSAv2<D, D::Top_Abstract_Level, E_StatsInfo::Search>;
	
}}}
