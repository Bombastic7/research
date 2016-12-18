#!/bin/python


from alg_dom_defs import *


def goodPair(dom, alg):
	return (not(alg[1]) or dom[2]) and (not(alg[2]) or dom[3])
		


print"""
/*
	Automatically generated file.
	
	There are {0} domain/algorithm pairs implemented.
	
*/""".format(len([() for a in ALGS for d in DOMS if goodPair(d, a)]))



print """
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


"""


print """
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


print """
void selectAll(Json const& jExecDesc) {
	std::string pDom = jExecDesc["domain"];
	std::string pAlg = jExecDesc["algorithm"];
	
	if(pDom == "")
		throw std::runtime_error("Bad domain");
"""

for (alg, dom) in [(a[0], d[0]) for a in ALGS for d in DOMS if goodPair(d, a)]:
	print """
	else if(pDom == "{0}" && pAlg == "{1}")
		mjon661::execRoutine<{0}, {1}>(jExecDesc);
	""".format(dom, alg)


print """
	else
		throw std::runtime_error("Bad domain or algorithm");
}
}//mjon661
"""




print """
void run_trial1(mjon661::Json const& jExecDescSet, std::string const& key) {
	
	mjon661::selectAll(jExecDescSet.at(key));
	
}
"""
