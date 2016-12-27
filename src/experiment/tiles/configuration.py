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
N_WORKERS = int(multiprocessing.cpu_count() * 1.5)

def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	
	return declStr



def makeAlgDomName(alg, dom):
	return alg["name"] + "_" + dom["name"]



def execWorker(algdomQueue, resDict, lck, probfile):
	with open(probfile) as f:
		probset = json.load(f)

	while True:
		ad = algdomQueue.get()
		if ad is None:
			break
		
		for (wf, wt) in ad["weights"]:


			probsetitems = probset.iteritems()
				
			for (k,pr) in probsetitems:
				
				execParams = {}
				execParams["name"] = ad["name"]
				
				execParams["domain conf"] = pr
				execParams["algorithm conf"] = {"wf" : wf, "wt" : wt}
				
				execParams["wf"] = wf
				execParams["wt"] = wt
				
				execParams["time limit"] = 25
				execParams["memory limit"] = 2100


				proc = subprocess.Popen(["./searcher", "-s"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
			
				searcherOut = proc.communicate(input=bytearray(json.dumps(execParams)))[0]

				res = json.loads(searcherOut)
				
				lck.acquire()
				
				
				if ad["name"] not in resDict:
					resDict[ad["name"]] = {}
				
				forName = resDict[ad["name"]]

				if str((wf, wt)) not in forName:
					forName[str((wf, wt))] = {}
				
				forName[str((wf, wt))][k] = res
				
				resDict[ad["name"]] = forName
				
				lck.release()
				
				with open(RES_CACHE_DIR + ad["name"] + "_" + str((wf, wt)) + "_" + k + ".json", "w") as f:
					json.dump(res, f, indent=4, sort_keys=True)






		



ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "abt" : False, "weights" : [(1,0)]},
		{"name" : "HAstar", "class" : "algorithm::hastargeneric::HAstar_StatsLevel", "abt" : True, "weights" : [(1,0)]},
		{"name" : "UGSAv3", "class" : "algorithm::ugsav3::UGSAv3_StatsLevel", "abt" : True, "weights" : [(1,0)]},
		]



DOMS = [
		{"name" : "base15", "class" : tiles_stack(3,3,False,True,0), "abt": False},
		{"name" : "abt15", "class" : tiles_stack(3,3,False,False,7), "abt": True},
		
		
		]



if __name__ == "__main__":
	if sys.argv[1] == "prob":
		gen_problems.genTilesProblemSet(3,3,10,"tiles8_probs_A.json")
	
	elif sys.argv[1] == "exec":
		if not os.path.exists(RES_CACHE_DIR):
			os.makedirs(RES_CACHE_DIR)
		
		inprobfile = sys.argv[2]
		outresfile = sys.argv[3]
		
		algdoms = [ { "alg" : a["class"], "dom" : d["class"], "name" : makeAlgDomName(a,d), "weights": a["weights"] } for a in ALGS for d in DOMS if a["abt"] == d["abt"] ]
		
		manager = multiprocessing.Manager()
		resultsDict = manager.dict()
		taskQueue = manager.Queue()
		dictLock = manager.Lock()
		
		for i in algdoms:
			taskQueue.put(i)
		
		for i in range(N_WORKERS):
			taskQueue.put(None)
		
		workers = [multiprocessing.Process(target = execWorker, args = (taskQueue, resultsDict, dictLock, inprobfile)) for i in range(N_WORKERS)]
		
		for i in workers:
			i.start()
		
		for i in workers:
			i.join()
		
		with open(outresfile, "w") as f:
			json.dump(resultsDict._getvalue(), f)
