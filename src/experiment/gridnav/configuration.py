#!/bin/python

import sys
import json

import gen_problems

def _bstr(b):
	return "true" if b else "false"


def gridnav_blocked(height, width, mv8, cstLC, hr):
	
	declStr = "gridnav::blocked::GridNav_DomainStack_single<{0}, {1}, {2}, {3}, {4}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), _bstr(hr))
	
	return declStr
	
	
	


def gridnav_blocked_stack_merge(height, width, mv8, cstLC, hfact, wfact, fillfact, maxAbtLvl = 1000):
	
	declStr = "gridnav::blocked::GridNav_DomainStack_MergeAbt<{0},{1},{2},{3},{mxL},{4},{5},{6}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), hfact, wfact, fillfact, mxL=maxAbtLvl)
	
	return declStr



def makeAlgDomName(alg, dom):
	return alg["name"] + "_" + dom["name"]



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





def executeProblemFile(algdoms):
	with open("gn6_probs_A.json") as f:
		probset = json.load(f)

	allRes = {}
	
	for ad in algdoms:
		adres = {}
		
		for (wf, wt) in ad["weights"]:
			weightres = {}

			for (k,pr) in probset.iteritems():
				
				execParams = {}
				execParams["name"] = ad["name"]
				
				execParams["domain conf"] = pr
				execParams["algorithm conf"] = {"wf" : wf, "wt" : wt}
				
				result = executeSearcher(execParams)
				
				weightres[k] = result 

			adres[str((wf,wt))] = weightres
		
		allRes[ad["name"]] = adres
	
	with open("gn6_results_A.json", "w") as f:
		json.dump(allRes, f, indent=4, sort_keys=True)







ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "abt" : False, "weights" : [(1,0)]},
		]



DOMS = [
		{"name" : "abtstack", "class" : gridnav_blocked_stack_merge(1000, 1000, True, True, 2, 2, 2, 1000), "abt": True},
		{"name" : "base", "class" : gridnav_blocked(1000, 1000, True, True, True), "abt": False},
		
		
		]



if __name__ == "__main__":
	if sys.argv[1] == "prob":
		gen_problems.genGridNavMap(1000, 1000, 0.4, "gn6_A")
		gen_problems.genGridNavProblemSet("gn6_probs_A.json", "gn6_A", 1000, 1000, 10, 0.5)

	elif sys.argv[1] == "exec":
		algdoms = [ { "alg" : a["class"], "dom" : d["class"], "name" : makeAlgDomName(a,d), "weights": a["weights"] } for a in ALGS for d in DOMS if a["abt"] == d["abt"] ]
		executeProblemFile(algdoms)
