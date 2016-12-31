#pragma once

#include <cmath>
#include "util/json.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {	

	using Util_t = float;
	using flt_t = float;
	
	
	template<typename = void>
	struct AlgoConf {
		
		AlgoConf(Json const& jConfig) {
			
			if(jConfig.count("use_all_frontier") && (bool)jConfig.at("use_all_frontier"))
				useAllFrontier = true;
			else
				useAllFrontier = false;
		
		
			mPref = std::round((double)jConfig.at("wt") / (double)jConfig.at("wf"));
		}
		
		
		bool useAllFrontier;
		unsigned mPref;
	};
	
}}}
