#!/bin/python

import sys
import os
import random
import json
import subprocess
import time
import multiprocessing
from multiprocessing.managers import SyncManager
import signal
from Queue import Empty

import gen_problems


RES_CACHE_DIR = "./rescache/"

WORKER_MEM = 30000
TIME_LIMIT = 3600



def setNWorkers():
	global N_WORKERS
	meminfo = dict((i.split()[0].rstrip(':'),int(i.split()[1])) for i in open('/proc/meminfo').readlines())
	mem_mib = meminfo['MemTotal'] / 1024
	N_WORKERS = max(1, mem_mib / WORKER_MEM)

#setNWorkers()

N_WORKERS = 2


def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	
	return declStr



def makeAlgDomName(alg, dom):
	return alg["name"] + "_" + dom["name"]



def execWorker(algdomprobQueue, resDict, statusMsgQueue, doTermValue, lck, probfile, doDump):
	logFileObj = open(RES_CACHE_DIR + str(os.getpid()) + ".out", "w")
	sys.stdout = logFileObj
	sys.stderr = logFileObj

	signal.signal(signal.SIGINT, signal.SIG_IGN)
	
	with open(probfile) as f:
		probset = json.load(f)

	while True:
		adp = algdomprobQueue.get()
		if adp is None:
			print "Finished"
			statusMsgQueue.put(str(os.getpid()) + ": finished")
			break
		
		cachedResult = None
		
		
		for (wf, wt) in adp["weights"]:

			probsetitems = probset.iteritems()
			
			probkey = adp["probkey"]
			probdesc = probset[probkey]
			

			execParams = {}
			execParams["name"] = adp["name"]
			
			execParams["domain conf"] = probdesc
			

			execParams["algorithm conf"] = adp["conf"]

			
			execParams["algorithm conf"]["wf"] = wf
			execParams["algorithm conf"]["wt"] = wt
			
			execParams["wf"] = wf
			execParams["wt"] = wt
			
			execParams["time limit"] = TIME_LIMIT
			execParams["memory limit"] = WORKER_MEM

			instanceName = adp["name"] + "_" + str((wf, wt)).replace(" ", "_") + "_" + probkey
			
			if doDump:
				res = execParams
			
			else:	
				
				print "starting", instanceName
				logFileObj.flush()
				
				
				
				if not adp["util_aware"] and cachedResult is not None:
					res = cachedResult
					print "using cached result"
					statusMsgQueue.put(str(os.getpid()) + ": " + instanceName + ": cached result")
				
				
				else:
					proc = subprocess.Popen(["./searcher", "-s", instanceName], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
					runningSubprocess = proc
					
					searcherOut = proc.communicate(input=bytearray(json.dumps(execParams)))[0]
					
					if doTermValue.value:
						statusMsgQueue.put(str(os.getpid()) + ": finished (signal)")
						return
					
					try:
						print "\n", searcherOut, "\n"
						res = json.loads(searcherOut)

						if res["_result"] == "good":
							res["util_real"] = res["_solution_cost"] * wf + res["_cputime"] * wt
							
							if "_base_expd" in res["_algorithm_report"]:
								baseExpd = res["_algorithm_report"]["_base_expd"]
								baseGend = res["_algorithm_report"]["_base_gend"]
							else:
								baseExpd = res["_algorithm_report"]["_all_expd"]
								baseGend = res["_algorithm_report"]["_all_gend"]
								
							res["util_base_expd"] = res["_solution_cost"] * wf + baseExpd * wt
							res["util_base_gend"] = res["_solution_cost"] * wf + baseGend * wt
							res["util_all_expd"] = res["_solution_cost"] * wf + res["_algorithm_report"]["_all_expd"] * wt
							res["util_all_gend"] = res["_solution_cost"] * wf + res["_algorithm_report"]["_all_gend"] * wt
					
						else:
							res["util_real"] = 0
							res["util_base_expd"] = 0
							res["util_base_gend"] = 0
							res["util_all_expd"] = 0
							res["util_all_gend"] = 0
						
						if res["_result"] != "good":
							statusMsgQueue.put(">> " + str(os.getpid()) + ": " + instanceName + " " + res["_result"])
						else:
							statusMsgQueue.put(str(os.getpid()) + ": " + instanceName)

						cachedResult = res
					
					except ValueError as e:
						print searcherOut, "^^"
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
				
				print e.__class__.__name__
				print e
				logFileObj.flush()
				
				statusMsgQueue.put(str(os.getpid()) + ": Exception: " +  e.__class__.__name__ + " " + str(e))
			
			lck.release()
			
			with open(RES_CACHE_DIR + instanceName + ".json", "w") as f:
				json.dump(res, f, indent=4, sort_keys=True)




def syncManagerInit():
	signal.signal(signal.SIGINT, signal.SIG_IGN)


		
WEIGHTS = [(1,0),(1,0.01),(1,0.1),(1,1),(1,10),(1,100),(0,1)]


ALGS = [
		{"name" : "Astar", "class" : "algorithm::Astar", "header" : "search/astar.hpp", "abt" : False, "util_aware" : False},
		#{"name" : "Bugsy", "class" : "algorithm::Bugsy", "header" : "search/bugsy.hpp", "abt" : False, "util_aware" : True},
		#{"name" : "Bugsy_norm", "class" : "algorithm::Bugsy", "conf" : {"normalised_exptime":True}, "header" : "search/bugsy.hpp",  "abt" : False, "util_aware" : True},
		{"name" : "HAstar", "class" : "algorithm::hastarv2::HAstar_StatsLevel", "header" : "search/hastar/v2/hastar.hpp", "abt" : True, "util_aware" : False},
		#{"name" : "HAstar1_nc", "class" : "algorithm::hastarv2::HAstar_StatsSimple", "conf" : {"do_caching":False}, "header" : "search/hastar/v2/hastar.hpp", "abt" : True, "util_aware" : False},
		
		#~ {"name" : "UGSAv4_st_hbfpairs", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
			#~ "conf" : {"use_all_frontier":False, "use_hbf_ref_init":False}},
		
		#{"name" : "UGSAv4_st_hbfinit_maxhd", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
		#	"conf" : {"uh_eval_mode":"Use_Max_H_D", "hbf_ref_init":True,  "g_for_hbf" : True}},
		
		#{"name" : "UGSAv4_st_hbfinit_evaluh", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
		#	"conf" : {"uh_eval_mode":"Use_H", "hbf_ref_init":True, "g_for_hbf" : True}},
		
		#{"name" : "UGSAv4_st_hbfinit_evalud", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
		#	"conf" : {"uh_eval_mode":"Use_D", "hbf_ref_init":True, "g_for_hbf" : True}},
		
		#~ {"name" : "UGSAv4_af_hbfpairs", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
			#~ "conf" : {"use_all_frontier":True, "use_hbf_ref_init":False}},
		
		#~ {"name" : "UGSAv4_af_hbfinit", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
			#~ "conf" : {"use_all_frontier":True, "use_hbf_ref_init":True}},
		
		#~ {"name" : "UGSAv4_st_hbfinit_g", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
			#~ "conf" : {"use_all_frontier":False, "use_hbf_ref_init":True, "use_g_for_hbf":True}},
		
		#~ {"name" : "UGSAv4_af_hbfinit_g", "class" : "algorithm::ugsav4::UGSAv4_StatsSimple", "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True,
			#~ "conf" : {"use_all_frontier":True, "use_hbf_ref_init":True, "use_g_for_hbf":True}},
		
		#{"name" : "UGSAv4_af", "class" : "algorithm::ugsav4::UGSAv4_StatsLevel", "conf" : {"use_all_frontier":True}, "header" : "search/ugsa/v4/ugsa_v4.hpp", "abt" : True, "util_aware" : True},
		#{"name" : "AstarExp", "class" : "algorithm::AstarExperiment", "header" : "search/astar_experiment.hpp", "abt" : False, "weights" : [(1,0)]},
		]



DOMS = [
		#{"name" : "8h_5", "class" : tiles_stack(3,3,False,True,5), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 8},
		{"name" : "15_7", "class" : tiles_stack(4,4,False,True,7), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 15},
		
		#{"name" : "8hw_5", "class" : tiles_stack(3,3,True,True,5), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 8},
		{"name" : "15w_7", "class" : tiles_stack(4,4,True,True,7), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 15}
		]


class AlgorithmInfo:
	lookup = {}
	
	def __init__(self, name, cls, hdr, isabt, isutilaware, conf = None):
		self.name = name
		self.cls = cls
		self.hdr = hdr
		self.isabt = isabt
		self.isutilaware = isutilware
		self.conf = conf
		AlgorithmInfo.lookup[name] = self


class DomainInfo:
	lookup = {}
	
	def __init__(self, name, cls, hdr, isabt, probcls):
		self.name = name
		self.cls = cls
		self.hdr = hdr
		self.isabt = isabt
		self.probcls = probcls
		DomainInfo.lookup[name] = self


class ProblemSetInfo:
	lookup = {}
	
	def __init__(self, probcls, name, func, args):
		self.probcls = probcls
		self.fname = fname
		self.func = func
		self.args = args
		ProblemSetInfo.lookup[name] = self
	
	def output(self):
		probs = self.func(self.args)

		with open(self.name, "w") as f:
			json.dump(probs, f)


class ExecutionInfo:
	def __init__(self, d, a, w, p):
		params = {}
		
		params["name"] = a.name + "_" + d.name
		params["domain conf"] = p
		
		if a.conf is not None:
			params["algorithm conf"] = a.conf
		else:
			params["algorithm conf"] = {}
		
		params["algorithm conf"]["wf"] = w[0]
		params["algorithm conf"]["wt"] = w[1]
				
		params["time limit"] = TIME_LIMIT
		params["memory limit"] = WORKER_MEM
		params["instance"] = params.name + "_" + str((wf, wt)).replace(" ", "_") + "_" + str(p)
		
		self.params = params
	
	def execute(self):
		try:
			proc = subprocess.Popen(["./searcher", "-s", instanceName], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
			searcherOut = proc.communicate(input=bytearray(json.dumps(self.params)))[0]
		
			self.results = json.load(searcherOut)

		except Exception as e:
			self.results = {"_result":"exception", "_error_what": e.__class__.__name__ + " " + str(e) }




def prepExecTable(doms, algs, weights, problems):
	execTable = []
	
	for di in range(len(doms)):
		execTable.append([])
		
		for ai in range(len(algs)):
			execTable[di].append([])
			
				for wi in range(len(weights)):
					execTable[di][ai].append([])
						
						for pi in range(len(problems)):
							
							t = ExecutionInfo(doms[di], algs[di], weights[wi], problems[pi])
							
							execTable[di][ai][wi].append((t)

	return execTable
	


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


		def getOrDefault(params, key):
			if key in params:
				return params[key]
			else:
				return {}


		algdomsprobs = [ { 	
						"alg" : a["class"], 
						"dom" : d["class"], 
						"name" : makeAlgDomName(a,d), 
						"weights": WEIGHTS,
						"util_aware" : a["util_aware"],
						"probkey": k,
						"conf" : getOrDefault(a, "conf")
						} 
						for a in ALGS for d in DOMS for k in probset.iterkeys()
						if not a["abt"] or d["abt"] and d["probcls"] == inprobcls ]
		
		print "Executing problems for", len(algdomsprobs), "alg/doms/probs"
		
		

		
		manager = SyncManager()
		manager.start(syncManagerInit)
		resultsDict = manager.dict()
		taskQueue = manager.Queue()
		statusMsgQueue = manager.Queue()
		doTerminateValue = manager.Value(bool, False)
		dictLock = manager.Lock()


		for i in algdomsprobs:
			taskQueue.put(i)
		
		for i in range(N_WORKERS):
			taskQueue.put(None)
		
		workers = [multiprocessing.Process(target = execWorker, args = (taskQueue, resultsDict, statusMsgQueue, doTerminateValue, dictLock, inprobfile, doDump)) for i in range(N_WORKERS)]
		
		def sigHandler(signum, frame):
			print str(os.getpid()) + ": caught signal " + str(signum)
			doTerminateValue.value = True
		
		signal.signal(signal.SIGINT, sigHandler)
		signal.signal(signal.SIGTERM, sigHandler)
		
		
		for i in workers:
			i.start()
		
		runningWorkers = N_WORKERS

		while runningWorkers > 0:
			for i in range(len(workers)):
				if workers[i] is not None:
					workers[i].join(1)
					
					if not workers[i].is_alive():
						workers[i] = None
						runningWorkers -= 1
						print "runningWorkers", runningWorkers

			while True:
				try:
					print statusMsgQueue.get_nowait()
				except Empty:
					break
			
			sys.stdout.flush()

		
		with open(outresfile, "w") as f:
			json.dump(resultsDict._getvalue(), f, indent=4, sort_keys=True)
