#!/bin/python

import os

import configuration as conf



def searcher_code(algdom):
	
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

#include "app/platform/resource.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/time.hpp"


#include "domain/tiles/fwd.hpp"
#include "domain/pancake/fwd.hpp"
#include "domain/gridnav/fwd.hpp"

#include "search/astar.hpp"
#include "search/idastar.hpp"
#include "search/bugsy.hpp"
#include "search/hastar/generic/hastar.hpp"
#include "search/ugsa/v1/ugsa_v1.hpp"
#include "search/ugsa/v2/ugsa_v2.hpp"
#include "search/ugsa/v2_bf/ugsa_v2.hpp"

"""


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

		
		D 		dom(jExecDesc.at("domain conf"));
		Alg<D> 	algo(dom, jExecDesc.at("algorithm conf"));
		
		Timer timer;
		Solution<BaseDomain> sol;
		
		Json jOut;
		bool memLimitReached = false;
		
		timer.start();
		
		try {
			algo.execute(sol);
			jOut["result"] = "good";
		
		} catch(std::bad_alloc const& e) {
			memLimitReached = true;
			exit(11);
		}
		
		//Cpu time limit reached raises a signal and exits the program via exit(10) (app/platform/resource.cc).
		
		timer.stop();
		
		
		jOut["solution length"] = sol.operators.size();
		jOut["solution cost"] = sol.pathCost(dom);
		jOut["algo"] = algo.report();
		jOut["resources"] = resourceReport();
		jOut["walltime"] = timer.seconds();
		
		
		double cputime = jOut["resources"].at("cputime");
		double walltime = timer.seconds();
		double exptimeCpu = cputime / jOut["algo"].at("expd").get<double>();
		double exptimeWall = walltime / jOut["algo"].at("expd").get<double>();
		
		double wf = jExecDesc.at("wf");
		double wt = jExecDesc.at("wt");
		
		double ucpu = wf * sol.pathCost(dom) + wt * cputime;
		double uwall = wf * sol.pathCost(dom) + wt * walltime;
		
		jOut["cputime"] = cputime;
		jOut["exptime_cpu"] = exptimeCpu;
		jOut["exptime_wall"] = exptimeWall;
		jOut["utility"] = ucpu;
		jOut["utility_wall"] = uwall;
		jOut["utility_cpu"] = ucpu;
		
		std::cout << jOut.dump(4) << "\\n";
		
		if(jExecDesc.count("print solution"))
			if(jExecDesc.at("print solution").get<bool>())
				sol.printSolution(dom, std::cout);
		
		if(memLimitReached && jExecDesc.count("memory limit dump")) {
			std::ofstream ofs("memlimitdump.txt", std::ofstream::in | std::ofstream::app);
			
			if(ofs)
				ofs << jExecDesc.dump(4) << "\\n" << jOut.dump(4) << "\\n";
		}
	}

"""


	codeStr += """
void selectAll(Json const& jExecDesc) {
	std::string pAlgDom = jExecDesc.at("algdom");
	
	if(pAlgDom == "")
		throw std::runtime_error("Bad domain or algorithm");
"""

	for i in algdom:
		codeStr += """
	else if(pAlgDom == "{0}")
		mjon661::execRoutine<{1}, {2}>(jExecDesc);
	""".format(i["name"], i["dom"], i["alg"])


	codeStr += """
	else
		throw std::runtime_error("Bad domain or algorithm");
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
		std::cout << "-s to get input from stdin\\n";
		std::cout << "execdesc.json for file input\\n";
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
		
		mjon661::Json j;
		
		ifs >> j;
		
		mjon661::selectAll(j);
	}

}
"""

	return codeStr
	




def genSearcher():
	
	algdoms = [ { "alg" : a["class"], "dom" : d["class"], "name" : conf.makeAlgDomName(a,d) } for a in conf.ALGS for d in conf.DOMS if a["abt"] == d["abt"] ]
	
	with open(os.path.dirname(os.path.abspath(__file__)) + "/searcher_auto.cc", "w") as f:
			f.write(searcher_code(algdoms))
	

if __name__ == "__main__":
	genSearcher()



