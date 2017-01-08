#pragma once

#include <string>
#include <cmath>
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav4 {	


	using flt_t = double;
	//using ucost_t = unsigned long;
	
	using ucost_t = double;
	
	
	const unsigned D_Search_Stack_Add = 200;
	const unsigned H_Search_Stack_Add = 100;
	
	
	/*
	template<typename = void>
	struct AlgoConf {
		
		enum { Use_Max_H_D, Use_H, Use_D };
		
		
		AlgoConf(Json const& jConfig) {
			
			//~ if(jConfig.count("use_all_frontier") && (bool)jConfig.at("use_all_frontier"))
				//~ useAllFrontier = true;
			//~ else
				//~ useAllFrontier = false;
		
			
			if(jConfig.count("hbf_ref_init") && (bool)jConfig.at("hbf_ref_init"))
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
			

			
			if(jConfig.count("uh_eval_mode")) {
				std::string s = jConfig.at("uh_eval_mode");
				
				if(s == "Use_Max_H_D") uh_eval_mode = Use_Max_H_D;
				else if(s == "Use_H") uh_eval_mode = Use_H;
				else if(s == "Use_D") uh_eval_mode = Use_D;
				else
					gen_assert(false);
			}
			else
				uh_eval_mode = Use_Max_H_D;
			
			
			if(jConfig.count("g_for_hbf") && (bool)jConfig.at("g_for_hbf"))
				g_for_hbf = true;
			else
				g_for_hbf = false;
		}
		
		Json report() {
			Json j;
			j["uh_eval_mode"] = uh_eval_mode;
			j["wf"] = wf;
			j["wt"] = wt;
			j["hbf_ref_init"] = hbf_ref_init;
			j["g_for_hbf"] = g_for_hbf;
			return j;
		}
		
		
		//bool useAllFrontier;
		bool hbf_ref_init;
		//bool use_g_for_hbf;
		//unsigned kpref;
		double wf,wt;
		int uh_eval_mode;
		bool g_for_hbf;
		
	};
	* */
	
}}}
