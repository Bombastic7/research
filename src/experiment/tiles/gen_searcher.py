#!/bin/python


def genSearcherCode(algdoms, headers):
	
	codeStr = ""
	
	codeStr += """
/*
	Automatically generated file.
	
	There are {0} domain/algorithm pairs implemented.
	
*/""".format(len(algdoms))



	codeStr += """
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <unistd.h>

#include "app/platform/resource.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/time.hpp"
"""

	for i in headers:
		codeStr += """
#include "{0}\"""".format(i)



	codeStr += """


namespace mjon661 {	

	void fillJsonResult(Json& jOut) {
		jOut["_sol_length"] = 0;
		jOut["_sol_cost"] = 0;
		jOut["_mem_used"] = 0;
		jOut["_walltime"] = 0;
		jOut["_cputime"] = 0;
		jOut["_base_expd"] = 0;		
		jOut["_all_expd"] = 0;
	}

	template<typename D, template<typename> typename Alg>
	void execRoutine(Json const& jExecDesc) {

		using BaseDomain = typename D::template Domain<0>;
		
		if(jExecDesc.count("_time_limit")) {
			double res = setCpuTimeLimit(jExecDesc.at("_time_limit").get<double>());
			if(res < 0)
				throw std::runtime_error("Failed to set time limit");
		}
		
		if(jExecDesc.count("_memory_limit")) {
			double res = setVirtMemLimit(jExecDesc.at("_memory_limit").get<double>());
			if(res < 0)
				throw std::runtime_error("Failed to set memory limit");
		}
		
		Json jOut;
		
		Timer timer;
		Solution<BaseDomain> sol;
			
		try {		
			D 		dom(jExecDesc.at("_domain_conf"));
			Alg<D> 	algo(dom, jExecDesc.at("_algorithm_conf"));

			timer.start();
			algo.execute(sol);
			timer.stop();
			
			jOut = Json();
			jOut["algo report"] = algo.report();
			jOut["resources"] = resourceReport();
			
			jOut["_result"] = "good";
			jOut["_sol_length"] = sol.operators.size();
			jOut["_sol_cost"] = sol.pathCost(dom);
			jOut["_mem_used"] = jOut.at("resources").at("max rss (MB)");
			jOut["_walltime"] = timer.seconds();
			jOut["_cputime"] = jOut.at("resources").at("cputime");
			
			if(jOut.at("resource report").count("_base_expd"))
				jOut["_base_expd"] = jOut.at("resource report").at("_base_expd");
			
			else
				jOut["_base_expd"] = jOut.at("resource report").at("_all_expd");
			
			jOut["_all_expd"] = jOut.at("resource report").at("_all_expd");
			
			
			std::cout << jOut.dump(4) << "\\n";

		} catch(std::bad_alloc const& e) {
			jOut["_result"] = "OOM";
			fillJsonResult(jOut);
			std::string msg = jOut.dump();
			write(1, msg.c_str(), msg.size());
			exit(0);
		} catch(std::exception const& e) {
			jOut["_result"] = "exec exception";
			fillJsonResult(jOut);
			std::string errwht(e.what());
			jOut["_error_what"] = errwht;
			
			std::string msg = jOut.dump();
			write(1, msg.c_str(), msg.size());
			exit(0);
		}
		
	}

"""


	codeStr += """
void selectAll(Json const& jExecDesc) {
	std::string pAlgDom = jExecDesc.at("_name");
	
	if(false)
		;
"""

	for i in algdoms:
		codeStr += """
	else if(pAlgDom == "{0}")
		mjon661::execRoutine<{1}, {2}>(jExecDesc);
	""".format(i[0], i[2], i[1])


	codeStr += """
	else {
		Json jErr;
		jErr["_result"] = "bad name";
		std::string msg = jErr.dump();
		write(1, msg.c_str(), msg.size());
		exit(0);
	}
}

"""



	codeStr += """
void run_prob_from_set(mjon661::Json const& jExecDescSet, std::string const& key) {
	
	mjon661::selectAll(jExecDescSet.at(key));
	
}
}//mjon661
"""

	codeStr += """
int main(int argc, const char* argv[]) {
	
	if(argc == 1) {
		std::cout << "Usage:\\n";
		std::cout << "-s                     | Get input from stdin\\n";
		std::cout << "<exec>                 | for file input\\n";
		std::cout << "<execset> <key>[...]   |\\n";
		return 2;
	
	}
	
	if(std::string(argv[1]) == "-s") {
		mjon661::Json j;
		std::cin >> j;
		mjon661::selectAll(j);
	}
	else {
		std::ifstream ifs(argv[1]);
		
		if(!ifs) {
			std::cout << "Could not open problem file\\n";
			return 2;
		}
	
		mjon661::Json jInput, jExec;
	
		ifs >> jInput;
		
		jExec = jInput;
		
		for(int i=2; i<argc; i++) {
			jExec = jExec.at(argv[i]);
		}
		
		mjon661::selectAll(jExec);
	}
}
"""

	return codeStr
	

