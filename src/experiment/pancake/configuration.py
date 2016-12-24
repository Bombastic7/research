#!/bin/python

import sys
import random
import json
import subprocess

def _bstr(b):
	return "true" if b else "false"


def pancake_stack_single(Ncakes, gapH):
	
	declStr = "pancake::Pancake_DomainStack_single<{0},{1}>".format(Ncakes, _bstr(gapH))

	return declStr
	


def pancake_stack_ignore(Ncakes, Abt1Sz, AbtStep):
	
	declStr = "pancake::Pancake_DomainStack_IgnoreAbt<{0},{1},{2}>".format(Ncakes, Abt1Sz, AbtStep)

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
	with open("pancake10_probs_A.json") as f:
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
	
	with open("pancake10_results_A.json", "w") as f:
		json.dump(allRes, f, indent=4, sort_keys=True)





def _getRandomPerm(sz):
	perm = range(0, sz)
	
	random.shuffle(perm)
	random.shuffle(perm)
	random.shuffle(perm)

	return tuple(perm)




def genPancakeProblemSet(sz, nprob, fname):
	probs = {}
	
	for i in range(0, nprob):
		probs[str(i)] = {"init" : _getRandomPerm(sz)}
	
	
	with open(fname, "w") as f:
		json.dump(probs, f, indent=4, sort_keys=True)






ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "abt" : False, "weights" : [(1,0)]},
		]



DOMS = [
		{"name" : "base", "class" : pancake_stack_single(10, True), "abt": True},
		{"name" : "abtstack", "class" : pancake_stack_ignore(10, 7, 1), "abt": False},
		]



if __name__ == "__main__":
	if sys.argv[1] == "prob":
		genPancakeProblemSet(10, 10, "pancake10_probs_A.json")

	elif sys.argv[1] == "exec":
		algdoms = [ { "alg" : a["class"], "dom" : d["class"], "name" : makeAlgDomName(a,d), "weights": a["weights"] } for a in ALGS for d in DOMS if a["abt"] == d["abt"] ]
		executeProblemFile(algdoms)
