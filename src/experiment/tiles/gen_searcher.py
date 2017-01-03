#!/bin/python

import os

import configuration as conf



def searcher_code(algdom, headers):
	
	codeStr = ""
	
	codeStr += """
/*
	Automatically generated file.
	
	There are {0} domain/algorithm pairs implemented.
	
*/""".format(len(algdom))



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
	template<typename D, template<typename> typename Alg>
	void execRoutine(Json const& jExecDesc) {

		using BaseDomain = typename D::template Domain<0>;
		
		if(jExecDesc.count("time limit")) {
			double res = setCpuTimeLimit(jExecDesc.at("time limit").get<double>());
			if(res < 0)
				throw std::runtime_error("Failed to set time limit");
		}
		
		if(jExecDesc.count("memory limit")) {
			double res = setVirtMemLimit(jExecDesc.at("memory limit").get<double>());
			if(res < 0)
				throw std::runtime_error("Failed to set memory limit");
		}
		
		Json jOut;
		
		Timer timer;
		Solution<BaseDomain> sol;
			
		try {		
			D 		dom(jExecDesc.at("domain conf"));
			Alg<D> 	algo(dom, jExecDesc.at("algorithm conf"));

			timer.start();
			algo.execute(sol);
			timer.stop();
			
			jOut["_result"] = "good";
		
			jOut["solution length"] = sol.operators.size();
			jOut["_solution_cost"] = sol.pathCost(dom);
			jOut["_algorithm_report"] = algo.report();
			jOut["resources"] = resourceReport();
			jOut["_walltime"] = timer.seconds();
			jOut["_cputime"] = jOut["resources"].at("cputime");
			
			std::cout << jOut.dump(4) << "\\n";
			
			if(jExecDesc.count("print solution"))
				if(jExecDesc.at("print solution").get<bool>())
					sol.printSolution(dom, std::cout);
			
		} catch(std::bad_alloc const& e) {
			jOut["_result"] = "OOM";
			std::string msg = jOut.dump();
			write(1, msg.c_str(), msg.size());
			exit(0);
		} catch(std::exception const& e) {
			jOut["_result"] = "exception";
			
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
	std::string pAlgDom = jExecDesc.at("name");
	
	if(false)
		;
"""

	for i in algdom:
		codeStr += """
	else if(pAlgDom == "{0}")
		mjon661::execRoutine<{1}, {2}>(jExecDesc);
	""".format(i["name"], i["dom"], i["alg"])


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
	




def genSearcher():
	
	algdoms = [ { "alg" : a["class"], "dom" : d["class"], "name" : conf.makeAlgDomName(a,d), "headers" : [d["header"], a["header"]] } for a in conf.ALGS for d in conf.DOMS if not a["abt"] or d["abt"] ]
	headers = list(set([j for s in [ i["headers"] for i in algdoms] for j in s]))
	
	
	with open(os.path.dirname(os.path.abspath(__file__)) + "/searcher_auto.cc", "w") as f:
			f.write(searcher_code(algdoms, headers))
	

if __name__ == "__main__":
	genSearcher()



