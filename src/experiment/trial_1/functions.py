#!/bin/python


import sys
import os
import json
import multiprocessing
import subprocess

import gen_domain_decls
import gen_searcher
import gen_problems



"""
gridnav = 1000x1000, Unit/LC
tiles8, tiles15, Unit/Weight


Weights = (1,0) (1, 0.1) (1, 10) (0, 1)

Astar, Bugsy, HAstar, UGSAv2(delay/bf)(1/multi level)

"""


ALG_DOM = [
		("algorithm::Astar", gen_domain_decls.gridnav_blocked(1000, 1000, True, True, True), "gridnav1000_1000", "Astar_gridnav"),
		("algorithm::Bugsy", gen_domain_decls.gridnav_blocked(1000, 1000, True, True, True), "gridnav1000_1000", "Bugsy_gridnav"),
		("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 3), "gridnav1000_1000", "hastar_gridnav"),
		("algorithm::ugsav2_bf::UGSAv2_StatsLevel",  gen_domain_decls.gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 3), "gridnav1000_1000", "ugsaBF_gridnav"),
		("algorithm::ugsav2::UGSAv2_StatsLevel",  gen_domain_decls.gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 3), "gridnav1000_1000", "ugsaDelay_gridnav"),
		
		#("algorithm::Astar", gen_domain_decls.pancake_stack_single(10, True), "pancake10", "Astar_pancakes"),
		
		#("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.pancake_stack_ignore(10, 9, 1), "pancake10", "HAstar_abtpancakes"),
		#("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.gridnav_blocked_stack_merge(10, 10, True, False, 2, 2, 3), "gridnav10_10", "hastar_gnabt"),
		#("algorithm::Astar", gen_domain_decls.tiles_stack(3, 3, False, True, 0), "tiles8", "Astar_tiles8"),
		#("algorithm::hastargeneric::HAstar_StatsLevel", gen_domain_decls.tiles_stack(3, 3, False, False, 5), "tiles8", "Astar_tiles8Abt")
		]


GEN_PROB_FILES = [ 
		
		{ "class" : "gridnav1000_1000", "fname" : "mapA", "gen" : "map", "blockedprob" : 0.45 },
		{ "class" : "gridnav1000_1000", "fname" : "gn_probsA.json", "gen" : "problems", "num" : 10, "mindistance" : 0.5, "map" : "problems/mapA" },
		#{ "class" : "pancake10", "fname" : "pc_probs.json", "num" : 10, "size" : 10 },
		#{ "class" : "tiles8", "fname" : "t8_probs.json", "num" : 10}
		
		]


EXEC_PROB_FILES = 	{

					"gridnav1000_1000" : ["gn_probsA.json"],
					"pancake10" : ["pc_probs.json"],
					"tiles8" : ["t8_probs.json"]
					
					}



WEIGHT_SCHEDULE = [(1,0), (1, 0.1), (1, 10), (0, 1)]



for i in GEN_PROB_FILES:
	i["fname"] = "./problems/" + i["fname"]


for i in EXEC_PROB_FILES.itervalues():
	for j in range(0, len(i)):
		i[j] = "./problems/" + i[j]
		



def executeProblem(execDesc):
	try:
		proc = subprocess.Popen(["./searcher", "-s"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
		
		searcherOut = proc.communicate(input=bytearray(json.dumps(execDesc)))[0]
		
		if proc.returncode == 0:
			res = json.loads(searcherOut)
			res["result"] = "success"
			return res
		
		elif proc.returncode == 10:
			res = {"result" : "time limit"}
			res["error_what"] = searcherOut
			return res
		
		elif proc.returncode == 11:
			res = {"result" : "memory limit"}
			res["error_what"] = searcherOut
			return res
	
		else:
			res = {"result" : "bad return code"}
			res["error_what"] = searcherOut
			return res
	
	except Exception as e:
		return json.loads("{ \"result\" : \"exception\", \"error_what\" : " + e + "\"}")
		

def _doExec(params):
	
	algdom = params["algdom"]
	key = params["key_desc"][0]
	desc = params["key_desc"][1]
	wf = params["weights"][0]
	wt = params["weights"][1]
				
	
	execDesc = {	"algdom" : algdom[3],
					"domain conf" : desc,
					"domain" : algdom[1],
					"algorithm" : algdom[0], 
					"algorithm conf" : {},
					"wf" : wf,
					"wt" : wt,
					"algorithm conf" : {"wf" : wf, "wt" : wt},
					"time limit" : 600,
					"memory limit" : 4000
			}
				
	return (key, executeProblem(execDesc))



def executeAllProblems():
	#import pdb; pdb.set_trace()
	workerPool = multiprocessing.Pool()
	
	for algdom in ALG_DOM:
		
		probcls = algdom[2]
		
		for probfile in EXEC_PROB_FILES[probcls]:
			
			with open(probfile) as f:
				probset = json.load(f)
			
			
			
			execDescList = [{"algdom" : algdom,
							 "key_desc" : key_desc,
							 "weights" : weights} for key_desc in probset.iteritems() for weights in WEIGHT_SCHEDULE]
	
	
			resList = workerPool.map(_doExec, execDescList)
			results = {}
			
			for (key, res) in resList:
				results[key] = res
			
			resFile = os.path.dirname(probfile) + "/"
			resFile += os.path.basename(probfile).split(".")[0] + "_" + algdom[3] + "_results.json"
			
			with open(resFile, "w") as f:
				
				json.dump(results, f, indent=4, sort_keys=True)
				print resFile
	
	



if __name__ == "__main__":

	func = sys.argv[1]

	if func == "searcher":
		gencode = gen_searcher.searcher_code(ALG_DOM)
	
		with open(os.path.dirname(os.path.abspath(__file__)) + "/searcher_auto.cc", "w") as f:
			f.write(gencode)
	
	elif func == "problems":
		if not os.path.exists("./problems"):
			os.makedirs("./problems")
		
		gen_problems.generateFiles(GEN_PROB_FILES)
		
	elif func == "exec":
		if not os.path.exists("./problems"):
			os.makedirs("./problems")
		
		executeAllProblems()
	
	else:
		raise RuntimeError()
	
	
	
