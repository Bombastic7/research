#!/bin/python

import sys
import os
import random
import json
import subprocess
import time
import multiprocessing

import gen_problems


RES_CACHE_DIR = "./rescache/"
MAX_SUBPROCS = multiprocessing.cpu_count()

def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	
	return declStr



def makeAlgDomName(alg, dom):
	return alg["name"] + "_" + dom["name"]




def putSubProc(execDesc, whch, subProcSlots):
	freeSlots = [i for i in range(len(subProcSlots)) if subProcSlots[i] is None]
	if len(freeSlots) == 0:
		return False
	
	newProc = subprocess.Popen(["./searcher", "-s"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
	newProc.stdin.write(json.dumps(execDesc))
	
	subProcSlots[freeSlots[0]] = (newProc, whch)
	return True


def getSubProc(subProcSlots):
	for i in range(len(subProcSlots)):
		if subProcSlots[i] is not None and subProcSlots[i][0].poll():
			searcherOut = subProcSlots[i][0].stdout.read()
			res = (json.loads(searcherOut), subProcSlots[i][1])
			subProcSlots[i] = None
			return res

	return None




def writeToCache(res, whch):
	
	fname = RES_CACHE_DIR + whch[0] + "_" + whch[1] + "_" + whch[2]

	with open(fname, "w") as f:
		json.dump(res, f, indent=4, sort_keys=True)




def executeProblemFile(algdoms, singleProbKey = None):
	with open("tiles15_probs_A.json") as f:
		probset = json.load(f)

	allRes = []
	subProcSlots = [None] * MAX_SUBPROCS
	
	for ad in algdoms:
		
		for (wf, wt) in ad["weights"]:

			if singleProbKey:
				probsetitems = [(singleProbKey, probset[singleProbKey])]
			else:
				probsetitems = probset.iteritems()
				
			for (k,pr) in probsetitems:
				
				execParams = {}
				execParams["name"] = ad["name"]
				
				execParams["domain conf"] = pr
				execParams["algorithm conf"] = {"wf" : wf, "wt" : wt}
				
				execParams["wf"] = wf
				execParams["wt"] = wt
				
				#execParams["time limit"] = 60
				#execParams["memory limit"] = 2100
				
				while True:
					res = getSubProc(subProcSlots)
					if res is not None:
						allRes.append(res)
						
						writeToCache(res)
						continue
				
					wasPut = putSubProc(execParams, (ad["name"], str((wf, wt)), k), subProcSlots)
					
					if wasPut:
						break
					
					time.sleep(5)

	
	resDict = {}
	
	for r in allRes:
		
		res = r[0]
		i0 = r[1][0]
		i1 = r[1][1]
		i2 = r[1][2]
		
		if i0 not in resDict:
			resDict[i0] = {}
		
		if i1 not in resDict[i0]:
			resDict[i0][i1] = {}
		
		resDict[i0][i1][i2] = res
		
	
	with open("tiles15_results_A_ugsa.json", "w") as f:
		json.dump(allRes, f, indent=4, sort_keys=True)







ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "abt" : False, "weights" : [(1,0)]},
		{"name" : "HAstar", "class" : "algorithm::hastargeneric::HAstar_StatsLevel", "abt" : True, "weights" : [(1,0)]},
		{"name" : "UGSAv3", "class" : "algorithm::ugsav3::UGSAv3_StatsLevel", "abt" : True, "weights" : [(1,0)]},
		]



DOMS = [
		{"name" : "base15", "class" : tiles_stack(4,4,False,True,0), "abt": False},
		{"name" : "abt15", "class" : tiles_stack(4,4,False,False,7), "abt": True},
		
		
		]



if __name__ == "__main__":
	if sys.argv[1] == "prob":
		gen_problems.genTilesProblemSet(4,4,10,"tiles15_probs_A.json")
	
	elif sys.argv[1] == "exec":
		if not os.path.exists(RES_CACHE_DIR):
			os.makedirs(RES_CACHE_DIR)
		
		algdoms = [ { "alg" : a["class"], "dom" : d["class"], "name" : makeAlgDomName(a,d), "weights": a["weights"] } for a in ALGS for d in DOMS if a["abt"] == d["abt"] ]
		
		if len(sys.argv) >= 3:
			algdoms[:] = [ad for ad in algdoms if ad["name"] == sys.argv[2]]
		
		assert(len(algdoms) >= 1)
		
		if(len(sys.argv) >= 4):
			executeProblemFile(algdoms, sys.argv[3])
		else:
			executeProblemFile(algdoms)
