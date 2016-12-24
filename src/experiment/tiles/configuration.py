#!/bin/python

import sys
import random
import json
import subprocess

import gen_problems

def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	
	return declStr



def makeAlgDomName(alg, dom):
	return alg["name"] + "_" + dom["name"]



def executeProblem(execDesc):
	res = {}
	
	try:
		proc = subprocess.Popen(["./searcher", "-s"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
		
		searcherOut = proc.communicate(input=bytearray(json.dumps(execDesc)))[0]	
		
		
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
	with open("tiles8_probs_A.json") as f:
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
				
				execParams["wf"] = wf
				execParams["wt"] = wt
				
				result = executeProblem(execParams)
				
				weightres[k] = result 

			adres[str((wf,wt))] = weightres
		
		allRes[ad["name"]] = adres
	
	with open("tiles8_results_A.json", "w") as f:
		json.dump(allRes, f, indent=4, sort_keys=True)







ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "abt" : False, "weights" : [(1,0)]},
		{"name" : "HAstar", "class" : "algorithm::hastargeneric::HAstar_StatsLevel", "abt" : True, "weights" : [(1,0)]},
		]



DOMS = [
		{"name" : "base", "class" : tiles_stack(3,3,True,True,0), "abt": False},
		{"name" : "abtstack", "class" : tiles_stack(3,3,True,False,5), "abt": True},
		
		
		]



if __name__ == "__main__":
	if sys.argv[1] == "prob":
		gen_problems.genTilesProblemSet(3,3,10,"tiles8_probs_A.json")
	
	elif sys.argv[1] == "exec":
		algdoms = [ { "alg" : a["class"], "dom" : d["class"], "name" : makeAlgDomName(a,d), "weights": a["weights"] } for a in ALGS for d in DOMS if a["abt"] == d["abt"] ]
		executeProblemFile(algdoms)
