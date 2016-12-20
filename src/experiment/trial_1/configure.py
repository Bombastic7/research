#!/bin/python


import sys

import gen_domain_decls



"""

Parameters:

Alg-Dom concrete classes
	algs, params
	doms, params
	how to pair
	
	create alg_dom_decls.hpp

	create searcher.cc
	

gen_problems.py
	doms -> probspec
	
	for each probspec
		nprobs
	
	create gen_problems.py



gen_runs.py
	location of problems

	get alg-dom classes
	
	other params
	
	create gen_runs.py


exec_runs.py
	run_set file
	results out file
	
	create exec_runs.py

"""



ALG_DOM = [
		("algorithm::Astar", gen_domain_decls.gridnav_blocked(10, 10, True, False, True), "gridnav10_10", "Astar_gridnav")
		]


GEN_PROB_FILES = [ 
		
		{ "class" : "gridnav10_10", "fname" : "mapA", "gen" : "map", "blockedprob" : 0.35 },
		{ "class" : "gridnav10_10", "fname" : "probsA", "gen" : "probs", "num" : 10, "mindistance" : 0.5 }
		
		]



def generateExecScript():
	
	






if __name__ == "__main__":

	func = sys.argv[1]

	if func == "searcher":
		gencode = gen_searcher.searcher_code(ALG_DOM)
	
		with open("searcher_auto.cc", "w") as f:
			f.write(gencode)
	
	elif func == "problems":
		gen_problems.generateFiles(GEN_PROB_FILES, "./problems")
		
	elif func == "exec_script":
		generateExecScript()
	
	else
		raise RuntimeError()
	
	
	














DOMS_TMPL + SETOFPARAMS = DOM_CONC, CAPS, PROBSPEC
DOMS -> PRO

DOM_CLASSES_TMPLS = (gendomdecls, 



ALG_DOM_CLASSES = (name, probspec)







ALG_CONCRETE_TMPLS = { 	"astar" : "algorithm::Astar<{0}>",
						"idastar" : "algorithm::IDAstar<{0}>",
						"HAstar" : "algorithm::hastargeneric::HAstar_StatsLevel<{0}>"
					}



