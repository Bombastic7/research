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
	
		else
			res = {"result" : "bad return code"}
			res["error_what"] = searcherOut
			return res
	
	except Exception as e:
		return json.loads("{ \"result\" : \"exception\", \"error_what\" : " + e + "\"}")
		



def executeAllProblems():
	
	def doExec(ex):
		return (ex[0], executeProblem(ex[1])

	workerPool = multiprocessing.Pool()
	
	for algdom in ALG_DOM:
		
		probcls = algdom[2]
		
		for probfile in os.listdir("./problems/" + probcls):
			
			with open(probfile) as f:
				probset = json.load(f)
			
			execDescList = [(*p, *w) for p in probset for w in WEIGHT_SCHEDULE]
			
			
			def doExec((key, desc, wf, wt)):
				
				execDesc = {	"domain conf" : desc,
								"domain" : algdom[1],
								"algorithm" : algdom[0], 
								"algorithm conf" : {},
								"wf" : wf,
								"wt" : wt,
								"algorithm conf" : {"wf" : wf, "wt" : wt},
								"time limit" : 60,
								"memory limit" : 2000
						}
				
				return (key, executeProblem(execDesc))
			
	
	
			resList = workerPool.map(doExec, execDescList)
			
			for (key, res) in resList:
				results[key] = res
			
			resFile = probfile + "_" + algdom[0] + "_" + algdom[1] + "_results.json"
			with open(resFile) as f:
				
				json.dump(results, f, indent=4, sort_keys=True)
				print resFile
	
	
			
			
			
				
	

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
		gen_problems.generateFiles(GEN_PROB_FILES)
		
	elif func == "exec":
		generateExecScript()
	
	else
		raise RuntimeError()
	
	
	
