#!/bin/python


import sys
import os
import json

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



def problemFilePath(cls, fname):
	return "./problems/" + cls + "/" + fname






ALG_DOM = [
		("algorithm::Astar", gen_domain_decls.gridnav_blocked(10, 10, True, False, True), "gridnav10_10", "Astar_gridnav")
		]


GEN_PROB_FILES = [ 
		
		{ "class" : "gridnav10_10", "fname" : "mapA", "gen" : "map", "blockedprob" : 0.35 },
		{ "class" : "gridnav10_10", "fname" : "probsA.json", "gen" : "probs", "num" : 10, "mindistance" : 0.5, "map" : "mapA" }
		
		]


EXEC_PROB_FILES = { "gridnav10_10" : ["probsA.json"] }



WEIGHT_SCHEDULE = [(1,0)]




def executeAllProblems():
	
	resultsFile = "results.json"
	
	for algdom in ALG_DOM:
		
		probcls = algdom[2]
		
		for probfile in os.listdir("./problems/" + probcls):
			
			with open(probfile) as f:
				probset = json.load(f)
			
			for (p, wf, wt) in [ (p, *w) for p in probset for w in WEIGHT_SCHEDULE]: #p should be (key, prob)
				
				execDesc = {	"domain conf" : p,
								"domain" : algdom[1], 
								"algorithm" : algdom[0], 
								"algorithm conf" : {},
								"wf" : wf,
								"wt" : wt, 
								"algorithm conf" : {"wf" : wf, "wt" : wt},
								"time limit" : 60,
								"memory limit" : 2000
						}
				
				#Open subprocess and pass execDesc via stdin pipe.
				#Make search routine into function so can use parallel map.
	
				#Get result json object, append to ./results/[probfile]_[domain].json. I think all algs output go in one file.

	try:
		ret = (key, json.loads(str(subprocess.check_output(["./searcher", RUNS_FILE, key]))))
	except subprocess.CalledProcessError as e:
		if e.returncode == 10:
			ret = (key, {"result" : "time limit"})
		elif e.returncode == 11:
			ret = (key, {"result" : "memory limit"})
		else:
			print "Unknown return code. Failed key:", key
			ret = (key, {"result" : "error", "error what" : str(e.returncode)})

	except:
		print "Unhandled exception. Failed key:", key
		ret = (key, { "result" : "error", "error what" : sys.exc_info()[0]})
	
	return ret






if __name__ == "__main__":

	func = sys.argv[1]

	if func == "searcher":
		gencode = gen_searcher.searcher_code(ALG_DOM)
	
		with open("searcher_auto.cc", "w") as f:
			f.write(gencode)
	
	elif func == "problems":
		gen_problems.generateFiles(GEN_PROB_FILES, problemFilePath)
		
	elif func == "exec":
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



