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
N_WORKERS = 7#int(multiprocessing.cpu_count() * 1.5)

def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	
	return declStr



def makeAlgDomName(alg, dom):
	return alg["name"] + "_" + dom["name"]



def execWorker(algdomQueue, resDict, lck, probfile, doDump):
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

				if doDump:
					res = execParams
				
				else:
					proc = subprocess.Popen(["./searcher", "-s"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
				
					searcherOut = proc.communicate(input=bytearray(json.dumps(execParams)))[0]

					try:
						res = json.loads(searcherOut)
					except ValueError as e:
						print searcherOut
						print e
						raise e
				
				lck.acquire()
				
				
				if ad["name"] not in resDict:
					resDict[ad["name"]] = {}
				
				forName = resDict[ad["name"]]

				if str((wf, wt)) not in forName:
					forName[str((wf, wt))] = {}
				
				forName[str((wf, wt))][k] = res
				
				resDict[ad["name"]] = forName
				
				lck.release()
				
				with open(RES_CACHE_DIR + ad["name"] + "_" + str((wf, wt)).replace(" ", "_") + "_" + k + ".json", "w") as f:
					json.dump(res, f, indent=4, sort_keys=True)






		



ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "abt" : False, "weights" : [(1,0)]},
		{"name" : "HAstar", "class" : "algorithm::hastargeneric::HAstar_StatsLevel", "abt" : True, "weights" : [(1,0)]},
		{"name" : "UGSAv3", "class" : "algorithm::ugsav3::UGSAv3_StatsLevel", "abt" : True, "weights" : [(1,0)]},
		]



DOMS = [
		{"name" : "base8", "class" : tiles_stack(3,3,False,True,0), "abt": False, "probcls" : 8},
		{"name" : "abt8", "class" : tiles_stack(3,3,False,False,5), "abt": True, "probcls" : 8},
		{"name" : "base15", "class" : tiles_stack(4,4,False,True,0), "abt": False, "probcls" : 15},
		{"name" : "abt15", "class" : tiles_stack(4,4,False,False,7), "abt": True, "probcls" : 15}
		]

PROBFILES = {
		8 : (3,3,10,"probs_8.json"),
		16 : (4,4,10,"probs_15.json")
			}



if __name__ == "__main__":
	if len(sys.argv) == 1:
		print "prob"
		print "exec <inprobs> <outresults> [dump]"
	
	if sys.argv[1] == "prob":
		for args in PROBFILES.itervalues():
			gen_problems.genTilesProblemSet(*args)
	
	
	elif sys.argv[1] == "exec":
		print "Searching with", N_WORKERS, "workers on", multiprocessing.cpu_count(), "processors."
		
		if not os.path.exists(RES_CACHE_DIR):
			os.makedirs(RES_CACHE_DIR)
		
		rmProc = subprocess.Popen(["rm", "-rf", RES_CACHE_DIR+"*"])
		rmProc.wait()
		
		
		inprobfile = sys.argv[2]
		outresfile = sys.argv[3]
		doDump = True if len(sys.argv) >= 5 and sys.argv[4] == "dump" else False
		
		inprobcls = None
		
		for (cls, (h, w, n, fn)) in PROBFILES.iteritems():
			if fn == inprobfile:
				inprobcls = cls
				break
		
		assert(inprobcls is not None)
		

		algdoms = [ { 	"alg" : a["class"], 
						"dom" : d["class"], 
						"name" : makeAlgDomName(a,d), 
						"weights": a["weights"] } 
						for a in ALGS for d in DOMS 
						if a["abt"] == d["abt"] and d["probcls"] == inprobcls ]
		
		manager = multiprocessing.Manager()
		resultsDict = manager.dict()
		taskQueue = manager.Queue()
		dictLock = manager.Lock()
		
		for i in algdoms:
			taskQueue.put(i)
		
		for i in range(N_WORKERS):
			taskQueue.put(None)
		
		workers = [multiprocessing.Process(target = execWorker, args = (taskQueue, resultsDict, dictLock, inprobfile, doDump)) for i in range(N_WORKERS)]
		
		for i in workers:
			i.start()
		
		for i in workers:
			i.join()
		
		with open(outresfile, "w") as f:
			json.dump(resultsDict._getvalue(), f, indent=4, sort_keys=True)
