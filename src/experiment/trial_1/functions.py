#!/bin/python


import sys
import os
import json
import multiprocessing
import subprocess

import gen_domain_decls
import gen_searcher
import gen_problems



GEN_DIR = "./generated/"


"""
alg: Name of algorithm class (used by gen_searcher)
dom: Name of domain class (used by gen_searcher)
class: problem class (used by generateRuns() )
name: nice name (used by gen_searcher, generateRuns() )
"""
ALG_DOM = [
		{
		"alg":		"algorithm::Astar", 
		"dom":		gen_domain_decls.gridnav_blocked(1000, 1000, True, True, True), 
		"class":	"gridnav1000_1000", 
		"name":		"Astar_gridnav"
		},]
"""
		{
		"alg":		"algorithm::Bugsy",
		"dom": 		gen_domain_decls.gridnav_blocked(1000, 1000, True, True, True),
		"class": 	"gridnav1000_1000",
		"name": 	"Bugsy_gridnav"
		},
		
		{
		"alg":		"algorithm::hastargeneric::HAstar_StatsLevel",
		"dom": 		gen_domain_decls.gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 3),
		"class": 	"gridnav1000_1000",
		"name": 	"hastar_gridnav"
		},
		
		
		
		{
		"alg":		"algorithm::Astar", 
		"dom":		gen_domain_decls.pancake_stack_single(10, True),
		"class": 	"pancake10",
		"name":		"Astar_pancake"
		},
		
		{
		"alg":		"algorithm::Bugsy", 
		"dom":		gen_domain_decls.pancake_stack_single(10, True), 
		"class": 	"pancake10", 
		"name":		"Bugsy_pancake"
		},
		
		{
		"alg":		"algorithm::hastargeneric::HAstar_StatsLevel", 
		"dom":		gen_domain_decls.pancake_stack_ignore(10, 8, 1), 
		"class": 	"pancake10", 
		"name":		"HAstar_pancake"
		},
		
		
		
		{"alg":		"algorithm::Astar", 
		"dom":		gen_domain_decls.tiles_stack(3,3,True,True,0), 
		"class": 	"tiles8", 
		"name":		"Astar_tiles8"
		},
		
		{
		"alg":		"algorithm::Bugsy", 
		"dom":		gen_domain_decls.tiles_stack(3,3,True,True,0), 
		"class": 	"tiles8", 
		"name":		"Bugsy_tiles8"
		},
		
		{
		"alg":		"algorithm::hastargeneric::HAstar_StatsLevel", 
		"dom":		gen_domain_decls.tiles_stack(3,3,True,False,5), 
		"class": 	"tiles8", 
		"name":		"HAstar_tiles8"
		},
	"""	
		#("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 3), "gridnav1000_1000", "hastar_gridnav"),
		#("algorithm::ugsav2_bf::UGSAv2_StatsLevel",  gen_domain_decls.gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 3), "gridnav1000_1000", "ugsaBF_gridnav"),
		#("algorithm::ugsav2::UGSAv2_StatsLevel",  gen_domain_decls.gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 3), "gridnav1000_1000", "ugsaDelay_gridnav"),
		
		#("algorithm::Astar", gen_domain_decls.pancake_stack_single(10, True), "pancake10", "Astar_pancakes"),
		
		#("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.pancake_stack_ignore(10, 9, 1), "pancake10", "HAstar_abtpancakes"),
		#("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.gridnav_blocked_stack_merge(10, 10, True, False, 2, 2, 3), "gridnav10_10", "hastar_gnabt"),
		#("algorithm::Astar", gen_domain_decls.tiles_stack(3, 3, False, True, 0), "tiles8", "Astar_tiles8"),
		#("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.tiles_stack(3, 3, False, False, 5), "tiles8", "Astar_tiles8Abt")
		#]


"""
Problem files that should be generated prior to performing searches.

type: general area this file belongs to (Used by gen_problems).
fname: file name of the generated problem set files (used by gen_problems).
num: number of problems to generate for the set (gen_problems).

Everything else is specific to the type and used by gen_problems.
"""
GEN_PROB_FILES = [ 
		
		{ "type" : "gridnav", "fname" : GEN_DIR+"mapA", "gen" : "map", "blockedprob" : 0, "dim" : (1000,1000) },
		{ "type" : "gridnav", "fname" : GEN_DIR+"gn_probsA.json", "gen" : "problems", "num" : 10, "dim" : (1000,1000), "mindistance" : 0.5, "map" : GEN_DIR+"mapA" },
		{ "type" : "pancake10", "fname" : GEN_DIR+"pancake10_probs.json", "num" : 10, "size" : 10 },
		{ "type" : "tiles8", "fname" : GEN_DIR+"t8_probs.json", "num" : 10}
		
		]


"""
Problem set files associated with ALG_DOM["class"], as generated from the info in GEN_PROB_FILES.

"""
EXEC_PROB_FILES = 	{

					"gridnav1000_1000" : [GEN_DIR+"gn_probsA.json"],
					"pancake10" : [GEN_DIR+"pc_probs.json"],
					"tiles8" : [GEN_DIR+"t8_probs.json"]
					
					}



WEIGHT_SCHEDULE = [(1,0)]#, (1, 0.1), (1, 10), (0, 1)]




def executeProblem(execDesc):
	try:
		proc = subprocess.Popen(["./searcher", "-s"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
		
		searcherOut = proc.communicate(input=bytearray(json.dumps(execDesc)))[0]
		
		res = {}
		
		if proc.returncode == 0:
			res = json.loads(searcherOut)
			res["status"] = "SUCCESS"
			return res
		
		elif proc.returncode == 10:
			res["status"] = "OOT"
			res["error_output"] = searcherOut
			return res
		
		elif proc.returncode == 11:
			res["status"] = "OOM"
			res["error_output"] = searcherOut
			return res
	
		else:
			res["status"] = "UNKNOWN"
			res["error_output"] = searcherOut + "\n\n" + str(proc.returncode)
			return res
	
	except Exception as e:
		res["status"] = "EXCEPTION"
		res["error_output"] = e

	return res

		


def _doExec(runDesc):
	
	key = runDesc[0]
	run = runDesc[1]

	
	execDesc = {	"algdom" : run["NAME"],
					"domain conf" : run["DOM_CONF"],
					"domain" : run["DOM"],
					"algorithm conf" : run["ALG_CONF"],
					"algorithm" : run["ALG"],
					"wf" : run["WF"],
					"wt" : run["WT"],
					"time limit" : 600,
					"memory limit" : 2000
			}
	
	res = executeProblem(execDesc)
	
	return (key, res)



def generateRuns(runsfile):

	allRuns = {}
	
	runkey = 0
	
	for algdom in ALG_DOM:
		
		probcls = algdom["class"]
		
		for probfile in EXEC_PROB_FILES[probcls]:
			
			with open(probfile) as f:
				probset = json.load(f)
			
			for weights in WEIGHT_SCHEDULE:
			
				for probdesc in probset.iteritems():
					run = {}
					run["ALG"] = algdom["alg"]
					run["DOM"] = algdom["dom"]
					run["WF"] = weights[0]
					run["WT"] = weights[1]
					run["NAME"] = algdom["name"]
					run["PROB_FILE"] = probfile
					run["PROB_KEY"] = probdesc[0]
					run["DOM_CONF"] = probdesc[1]
					
					if "conf" in algdom:
						run["ALG_CONF"] = algdom["conf"]
					else:
						run["ALG_CONF"] = {}
					
					run["ALG_CONF"]["wf"] = weights[0]
					run["ALG_CONF"]["wt"] = weights[1]
					
					allRuns[str(runkey)] = run
					runkey += 1
	
	with open(runsfile, "w") as f:
		json.dump(allRuns, f, indent=4, sort_keys=True)
	
	


def executeRuns(runsfile, resultsfile):
	
	with open(runsfile) as f:
		allRuns = json.load(f)
	
	
	workerPool = multiprocessing.Pool()
	
	results = workerPool.map(_doExec, allRuns.iteritems())


	with open(resultsfile, "w") as f:
		json.dump(dict(results), f, indent=4, sort_keys=True)

	



if __name__ == "__main__":

	func = sys.argv[1]

	if func == "searcher":
		gencode = gen_searcher.searcher_code(ALG_DOM)
	
		with open(os.path.dirname(os.path.abspath(__file__)) + "/searcher_auto.cc", "w") as f:
			f.write(gencode)
	
	elif func == "problems":
		if not os.path.exists(GEN_DIR):
			os.makedirs(GEN_DIR)
		
		gen_problems.generateFiles(GEN_PROB_FILES)
	
	elif func == "genruns":
		if not os.path.exists(GEN_DIR):
			os.makedirs(GEN_DIR)
		
		generateRuns(GEN_DIR+"runs.json")

	elif func == "exec":
		executeRuns(GEN_DIR+"runs.json", GEN_DIR+"results.json")
	
	else:
		raise RuntimeError()
	
	
	
