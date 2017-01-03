#pragma once

#include <string>
#include <cmath>
#include "util/json.hpp"
#include "util/exception.hpp"


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
			
			
			if(jConfig.count("hbf_cache")) {
				std::string cacheMethod = jConfig.at("hbf_cache");
				
				if(cacheMethod == "lin")
					cache_lin_period = jConfig.at("cache_lin_period");
				
				else if(cacheMethod == "geo") {
					cache_geo_init = jConfig.at("cache_geo_init");
					cache_geo_grow = jConfig.at("cache_geo_grow");
				}
				
				else
					throw ConfigException("Bad hbf cache method");
			}
		}
		
		
		bool useAllFrontier;
		bool use_hbf_ref_init;
		unsigned kpref;
		bool do_hbf_cache;
		bool is_cache_lin;
		unsigned cache_lin_period;
		unsigned cache_geo_init;
		unsigned cache_geo_grow;
		
	};
	
}}}
