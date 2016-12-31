#pragma once

#include <cmath>
#include "util/json.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {	

	using Util_t = float;
	using flt_t = float;
	using ucost_t = unsigned long;
	
	
	template<typename = void>
	struct AlgoConf {
		
		AlgoConf(Json const& jConfig) {
			
			if(jConfig.count("use_all_frontier") && (bool)jConfig.at("use_all_frontier"))
				useAllFrontier = true;
			else
				useAllFrontier = false;
		
			
			if(jConfig.count("use_hbf_ref_init") && (bool)jConfig.at("use_hbf_ref_init"))
				use_hbf_ref_init = true;
			else
				use_hbf_ref_init = false;
			
			
			kpref = std::round((double)jConfig.at("wt") / (double)jConfig.at("wf"));
			
			
		}
		
		
		bool useAllFrontier;
		bool use_hbf_ref_init;
		unsigned kpref;
	};
	
}}}
