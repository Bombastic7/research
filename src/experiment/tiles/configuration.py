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

WORKER_MEM = 3000
TIME_LIMIT = 60

def setNWorkers():
	global N_WORKERS
	meminfo = dict((i.split()[0].rstrip(':'),int(i.split()[1])) for i in open('/proc/meminfo').readlines())
	mem_mib = meminfo['MemTotal'] / 1024
	N_WORKERS = max(1, mem_mib / WORKER_MEM)

setNWorkers()

def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	
	return declStr



def makeAlgDomName(alg, dom):
	return alg["name"] + "_" + dom["name"]



def execWorker(algdomprobQueue, resDict, lck, probfile, doDump):
	logFileObj = open(RES_CACHE_DIR + str(os.getpid()) + ".out", "w")
	sys.stdout = logFileObj
	sys.stderr = logFileObj
	
	
	with open(probfile) as f:
		probset = json.load(f)

	while True:
		adp = algdomprobQueue.get()
		if adp is None:
			print "Finished"
			break
		
		cachedResult = None
		
		
		for (wf, wt) in adp["weights"]:

			probsetitems = probset.iteritems()
			
			probkey = adp["probkey"]
			probdesc = probset[probkey]
			

			execParams = {}
			execParams["name"] = adp["name"]
			
			execParams["domain conf"] = probdesc
			execParams["algorithm conf"] = {"wf" : wf, "wt" : wt}
			
			execParams["wf"] = wf
			execParams["wt"] = wt
			
			execParams["time limit"] = TIME_LIMIT
			execParams["memory limit"] = WORKER_MEM

			if doDump:
				res = execParams
			
			else:
				print "starting", adp["name"], {"wf" : wf, "wt" : wt}, probfile, probkey
				logFileObj.flush()
				
				if not adp["util_aware"] and cachedResult is not None:
					res = cachedResult
					print "using cached result"
				
				else:
					proc = subprocess.Popen(["./searcher", "-s"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
				
					searcherOut = proc.communicate(input=bytearray(json.dumps(execParams)))[0]

					try:
						res = json.loads(searcherOut)
						cachedResult = res
					
					except ValueError as e:
						print searcherOut
						print e
						logFileObj.flush()
						raise e
			
			lck.acquire()
			
			try:
				print "Writing", adp["name"], {"wf" : wf, "wt" : wt}, probfile, probkey
				logFileObj.flush()
				
				if adp["name"] not in resDict:
					resDict[adp["name"]] = {}
				
				forName = resDict[adp["name"]]

				if str((wf, wt)) not in forName:
					forName[str((wf, wt))] = {}
				
				forName[str((wf, wt))][probkey] = res
				
				resDict[adp["name"]] = forName
				print "Wrote", adp["name"], {"wf" : wf, "wt" : wt}, probfile, probkey
				logFileObj.flush()
				
			except Exception as e:
				print adp["name"] + "_" + str((wf, wt)).replace(" ", "_") + "_" + k
				print e.__class__.__name__
				print e
				logFileObj.flush()
			
			lck.release()
			
			with open(RES_CACHE_DIR + adp["name"] + "_" + str((wf, wt)).replace(" ", "_") + "_" + probkey + ".json", "w") as f:
				json.dump(res, f, indent=4, sort_keys=True)






		
WEIGHTS = [(1,0),(1,1),(1,5),(1,10), (1,100), (1,1000)]


ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "header" : "search/astar.hpp", "abt" : False, "util_aware" : False},
		{"name" : "Bugsy", "class" : "algorithm::Bugsy", "header" : "search/bugsy.hpp", "abt" : False, "util_aware" : True},
		{"name" : "HAstar1", "class" : "algorithm::hastarv2::HAstar_StatsLevel", "header" : "search/hastar/v2/hastar.hpp", "abt" : True, "util_aware" : False},
		{"name" : "HAstar1_nc", "class" : "algorithm::hastarv2::HAstar_StatsLevel", "conf" : {"do_caching":False}, "header" : "search/hastar/v2/hastar.hpp", "abt" : True, "util_aware" : False},
		{"name" : "UGSAv4_st", "class" : "algorithm::ugsav4::UGSAv4_StatsLevel", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True},
		{"name" : "UGSAv4_af", "class" : "algorithm::ugsav4::UGSAv4_StatsLevel", "conf" : {"use_all_frontier":True}, "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True},
		#{"name" : "AstarExp", "class" : "algorithm::AstarExperiment", "header" : "search/astar_experiment.hpp", "abt" : False, "weights" : [(1,0)]},
		]



DOMS = [
		{"name" : "8h_5", "class" : tiles_stack(3,3,False,True,5), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 8},
		#{"name" : "15h_7", "class" : tiles_stack(4,4,False,True,7), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 15},
		
		{"name" : "8hw_5", "class" : tiles_stack(3,3,True,True,5), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 8},
		#{"name" : "15hw_7", "class" : tiles_stack(4,4,True,True,7), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 15}
		]

PROBFILES = {
		8 : (3,3,3,"probs_8.json"),
		15 : (4,4,3,"probs_15.json")
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
		
		rmProc = subprocess.Popen(["rm", "-rf", RES_CACHE_DIR])
		rmProc.wait()

		os.makedirs(RES_CACHE_DIR)
		
		
		inprobfile = sys.argv[2]
		outresfile = sys.argv[3]
		doDump = True if len(sys.argv) >= 5 and sys.argv[4] == "dump" else False
		
		inprobcls = None
		
		for (cls, (h, w, n, fn)) in PROBFILES.iteritems():
			if fn == inprobfile:
				inprobcls = cls
				break
		
		assert(inprobcls is not None)
		
		
		with open(inprobfile) as f:
			probset = json.load(f)

		algdomsprobs = [ { 	
						"alg" : a["class"], 
						"dom" : d["class"], 
						"name" : makeAlgDomName(a,d), 
						"weights": WEIGHTS,
						"util_aware" : a["util_aware"],
						"probkey": k
						} 
						for a in ALGS for d in DOMS for k in probset.iterkeys()
						if not a["abt"] or d["abt"] and d["probcls"] == inprobcls ]
		
		print "Executing problems for", len(algdomsprobs), "alg/doms/probs"
		
		

		
		manager = multiprocessing.Manager()
		resultsDict = manager.dict()
		taskQueue = manager.Queue()
		dictLock = manager.Lock()
		
		for i in algdomsprobs:
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
