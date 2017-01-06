#pragma once

#include <string>
#include <cmath>
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {	


	using flt_t = double;
	//using ucost_t = unsigned long;
	
	using ucost_t = double;
	
	template<typename = void>
	struct AlgoConf {
		
		AlgoConf(Json const& jConfig) {
			
			//~ if(jConfig.count("use_all_frontier") && (bool)jConfig.at("use_all_frontier"))
				//~ useAllFrontier = true;
			//~ else
				//~ useAllFrontier = false;
		
			
			if(jConfig.count("use_hbf_ref_init") && (bool)jConfig.at("use_hbf_ref_init"))
				hbf_ref_init = true;
			else
				hbf_ref_init = false;
			
			//~ if(jConfig.count("use_g_for_hbf") && (bool)jConfig.at("use_g_for_hbf"))
				//~ use_g_for_hbf = true;
			//~ else
				//~ use_g_for_hbf = false;
			
			//kpref = std::round((double)jConfig.at("wt") / (double)jConfig.at("wf"));
			
			wf = jConfig.at("wf");
			wt = jConfig.at("wt");
			
			//~ if(jConfig.count("hbf_cache")) {
				//~ std::string cacheMethod = jConfig.at("hbf_cache");
				
				//~ if(cacheMethod == "lin")
					//~ cache_lin_period = jConfig.at("cache_lin_period");
				
				//~ else if(cacheMethod == "geo") {
					//~ cache_geo_init = jConfig.at("cache_geo_init");
					//~ cache_geo_grow = jConfig.at("cache_geo_grow");
				//~ }
				
				//~ else
					//~ throw ConfigException("Bad hbf cache method");
			//~ }
		}
		
		
		//bool useAllFrontier;
		bool hbf_ref_init;
		//bool use_g_for_hbf;
		//unsigned kpref;
		double wf,wt;
		//bool do_hbf_cache;
		//bool is_cache_lin;
		//unsigned cache_lin_period;
		//unsigned cache_geo_init;
		//unsigned cache_geo_grow;
		
	};
	
}}}
