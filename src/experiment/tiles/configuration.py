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

setNWorkers()

#N_WORKERS = 2


def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	
	return declStr


		


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
	
	def generate(self):
		self.problems = self.func(self.args)
	
	def load(self):
		with open(self.name) as f:
			self.problems = json.load(f)
	
	def dump(self):
		with open(self.name, "w") as f:
			json.dump(self.problems, f)



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




def prepExecObjs(outdict, doms, algs, weights, problems):
	for di in range(len(doms)):
		for ai in range(len(algs)):
				for wi in range(len(weights)):
						for pi in range(len(problems)):
							assert((di,ai,wi,pi) not in outdict)
							outdict[(di,ai,wi,pi)] = ExecutionInfo(doms[di], algs[di], weights[wi], problems[pi])


def flatExecArrayToNice(indict, doms, algs, weights, problems):
	outdict = {}
	
	for di in range(len(doms)):
		for ai in range(len(algs)):
			for wi in range(len(weights)):
				for pi in range(len(problems)):
					if doms[di].name not in outdict:
						outdict[doms[di].name] = {}
						
						if algs[ai].name not in outdict[doms[di].name]:
							outdict[doms[di].name] = {}
							
						weightKey = str(weights[wi])
							
						if weightKey not in outdict[doms[di].name][algs[ai].name]
							outdict[doms[di].name][algs[ai].name][weightKey] = {}
							
						assert(str(pi) not in outdict[doms[di].name][algs[ai].name][weightKey])
							
						outdict[doms[di].name][algs[ai].name][weightKey][str(pi)] = indict[(di,ai,wi,pi)]
	
	return outdict



def workerRoutine(sharedExecList, taskQueue, msgQueue):
	msgpfx = str(os.getpid()) + ": "
	
	try:
		while True:
			execKey = taskQueue.get()

			if execObj is None:
				msgQueue.put(msgpfx + "finished")
				return

			execObj = sharedExecList[execKey]
			
			msgQueue.put(msgpfx + "Starting " + execObj.params["instance"])
			execObj.execute()
			sharedExecList[execKey] = execObj
			
	except Exception as e:
		msgQueue(msgpfx + "Exception: " + e.__class__.__name__ + ": " + str(e))

			
		
		




ALGS = [
		AlgorithmInfo("Astar", "algorithm::Astar", "search/astar.hpp", False, False),
		AlgorithmInfo("HAstar", "algorithm::hastarv2::HAstar_StatsLevel", "search/hastar/v2/hastar.hpp", True, False),
		

		#{"name" : "Astar", "class" : "algorithm::Astar", "header" : "search/astar.hpp", "abt" : False, "util_aware" : False},
		#{"name" : "Bugsy", "class" : "algorithm::Bugsy", "header" : "search/bugsy.hpp", "abt" : False, "util_aware" : True},
		#{"name" : "Bugsy_norm", "class" : "algorithm::Bugsy", "conf" : {"normalised_exptime":True}, "header" : "search/bugsy.hpp",  "abt" : False, "util_aware" : True},
		#{"name" : "HAstar", "class" : "algorithm::hastarv2::HAstar_StatsLevel", "header" : "search/hastar/v2/hastar.hpp", "abt" : True, "util_aware" : False},
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



DOMS =	[
		DomainInfo("8h_7", tiles_stack(3,3,False,True,5), "domain/tiles/fwd.hpp", True, 8),
		DomainInfo("8hw_7", tiles_stack(3,3,True,True,5), "domain/tiles/fwd.hpp", True, 8),
		
		#{"name" : "8h_5", "class" : tiles_stack(3,3,False,True,5), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 8},
		#{"name" : "15_7", "class" : tiles_stack(4,4,False,True,7), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 15},
		
		#{"name" : "8hw_5", "class" : tiles_stack(3,3,True,True,5), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 8},
		#{"name" : "15w_7", "class" : tiles_stack(4,4,True,True,7), "header" : "domain/tiles/fwd.hpp", "abt": True, "probcls" : 15}
		]

PROBLEM_SETS =	[
				ProblemSetInfo(8, "tiles_8.json", gen_problems.genTilesProblemSet, (3, 3, 5)),
				ProblemSetInfo(15, "tiles_15.json", gen_problems.genTilesProblemSet, (4, 4, 5)),
				]

WEIGHTS = [(1,0),(1,0.01),(1,0.1),(1,1),(1,10),(1,100),(0,1)]



if __name__ == "__main__":
	if len(sys.argv) == 1:
		print "genprob <probset>"
		print "dump <probset> <outresults>"
		print "exec <probset> <outresults>"
	
	if sys.argv[1] == "genprob":
		ProblemSetInfo.lookup[sys.argv[2]].dump()
	
	elif sys.argv[1] == "dump":
		outfile = sys.argv[3]
		params = {}
		probset = ProblemSetInfo.lookup[sys.argv[2]]
		probset.load()
		prepExecObjs(params, DOMS, ALGS, WEIGHTS, probset.problems)
		
		outdict = flatExecArrayToNice(params, DOMS, ALGS, WEIGHTS, probset.problems)
		
		with open(outfile, "w") as f:
			json.dump(outdict, f)
	
	
	elif sys.argv[1] == "exec":
		outfile = sys.argv[3]

		probset = ProblemSetInfo.lookup[sys.argv[2]]
		probset.load()
		
		manager = multiprocessing.Manager()
		sharedExecList = manager.dict()
		taskQueue = manager.Queue()
		msgQueue = manager.Queue()
		
		prepExecObjs(sharedExecList, DOMS, ALGS, WEIGHTS, probset.problems)
		
		for k in sharedExecList.iterkeys():
			taskQueue.put(k)
		
		workers = [multiprocessing.Process(target = workerRoutine, args = (sharedExecList, taskQueue, msgQueue)) for i in range(N_WORKERS)]
			
		for i in range(N_WORKERS):
			taskQueue.put(None)
		
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
					print msgQueue.get_nowait()
				except Empty:
					break
			
			sys.stdout.flush()
		
		niceResults = flatExecArrayToNice(sharedExecList, DOMS, ALGS, WEIGHTS, probset.problems)

		with open(outfile, "w") as f:
			json.dump(niceResults, f, indent=4, sort_keys=True)

