#!/bin/python

import sys
import os
import itertools
import random
import json
import subprocess
import time
import multiprocessing
from collections import defaultdict
#import MySQLdb
from multiprocessing.managers import SyncManager
from Queue import Empty

import gen_searcher
import gen_tiles_problems
import gen_pancake_problems
import gen_gridnav_problems


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


#From itertools recipes. https://docs.python.org/2/library/itertools.html
def unique_everseen(iterable, key=None):
    "List unique elements, preserving order. Remember all elements ever seen."
    # unique_everseen('AAAABBBCCDAABBB') --> A B C D
    # unique_everseen('ABBCcAD', str.lower) --> A B C D
    seen = set()
    seen_add = seen.add
    if key is None:
        for element in itertools.ifilterfalse(seen.__contains__, iterable):
            seen_add(element)
            yield element
    else:
        for element in iterable:
            k = key(element)
            if k not in seen:
                seen_add(k)
                yield element


def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	return declStr


def pancake_stack_ignore(Ncakes, Abt1Sz, AbtStep):
	declStr = "pancake::Pancake_DomainStack_IgnoreAbt<{0},{1},{2}>".format(Ncakes, Abt1Sz, AbtStep)
	return declStr	

def pancake_stack_single(Ncakes, gapH):
	declStr = "pancake::Pancake_DomainStack_single<{0},{1}>".format(Ncakes, _bstr(gapH))
	return declStr
	
def gridnav_blocked(height, width, mv8, cstLC, hr):
	declStr = "gridnav::blocked::GridNav_DomainStack_single<{0}, {1}, {2}, {3}, {4}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), _bstr(hr))
	return declStr

def gridnav_blocked_stack_merge(height, width, mv8, cstLC, hfact, wfact, fillfact, maxAbtLvl = 1000):
	declStr = "gridnav::blocked::GridNav_DomainStack_MergeAbt<{0},{1},{2},{3},{mxL},{4},{5},{6}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), hfact, wfact, fillfact, mxL=maxAbtLvl)
	return declStr





class AlgorithmInfo:
	lookup = {}
	
	def __init__(self, name, cls, hdr, isabt, isutilaware, conf = None):
		self.name = name
		self.cls = cls
		self.hdr = hdr
		self.isabt = isabt
		self.isutilaware = isutilaware
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
		self.name = name
		self.func = func
		self.args = args
		ProblemSetInfo.lookup[name] = self
	
	def generate(self):
		self.problems = self.func(*self.args)
	
	def load(self):
		with open(self.name) as f:
			self.problems = json.load(f)
	
	def dump(self):
		with open(self.name, "w") as f:
			json.dump(self.problems, f, indent=4, sort_keys=True)



class ExecutionInfo:
	lookup = {}
	
	def __init__(self, execKey, d, a, w, p, pi):
		self.params = {}

		self.params["_domain"] = d.name
		self.params["_algorithm"] = a.name
		self.params["_name"] = a.name + "_" + d.name
		self.params["_domain_conf"] = p
		
		if a.conf is not None:
			self.params["_algorithm_conf"] = a.conf.copy()
		else:
			self.params["_algorithm_conf"] = {}
		
		self.params["_algorithm_conf"]["wf"] = w[0]
		self.params["_algorithm_conf"]["wt"] = w[1]
		self.params["_time_limit"] = TIME_LIMIT
		self.params["_memory_limit"] = WORKER_MEM
		self.params["instance"] = self.params["_name"] + "_" + str(w).replace(" ", "_") + "_" + str(pi)
		self.params["exec_key"] = execKey
		
		self.results = {"pre"}
		lookup[execKey] = self

		
	def computeUtils():
		if self.results["_result"] == "good":
			self.results["_util_real"] = self.results["_sol_cost"] * self.weights[0] + self.results["_cputime"] * self.weights[1]
			self.results["_util_norm_base"] = self.results["_sol_cost"] * self.weights[0] + self.results["_base_expd"]
			self.results["_util_norm_all"] = self.results["_sol_cost"] * self.weights[0] + self.results["_all_expd"]





def workerRoutine(sharedExecList, sharedResList, taskQueue, msgQueue):
	msgpfx = str(os.getpid()) + ": "
	
	try:
		while True:
			execKey = taskQueue.get()

			if execKey is None:
				msgQueue.put(msgpfx + "finished")
				return

			execParams = sharedExecList[execKey]
			
			msgQueue.put(msgpfx + "Starting " + execParams["instance"])
			
			try:
				proc = subprocess.Popen(["./searcher", "-s", execParams["instance"]], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
				searcherOut = proc.communicate(input=bytearray(json.dumps(execParams)))[0]
			
				sharedResList[execKey] = json.loads(searcherOut)

			except Exception as e:
				sharedResList[execKey] = {"_result":"exception", "_error_what": e.__class__.__name__ + " " + str(e) }
			

	except Exception as e:
		msgQueue.put(msgpfx + "Exception: " + e.__class__.__name__ + ": " + str(e))

			
	

def executeParallelSearches(execLst):
	manager = multiprocessing.Manager()
	sharedExecList = manager.list()
	sharedResList = manager.list()
	taskQueue = manager.Queue()
	msgQueue = manager.Queue()

	for i in execLst:
		sharedExecList.append(i.params)

	sharedResList = [None] * len(execLst)

	for i in range(len(execLst)):
		taskQueue.put(i)
	
	for i in range(N_WORKERS):
		taskQueue.put(None)
	
	workers = [multiprocessing.Process(target = workerRoutine, args = (sharedExecList, sharedResList, taskQueue, msgQueue)) for i in range(N_WORKERS)]

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
	
	for i in range(len(execLst)):
		execLst[i].results = sharedResLst[i]



ALGS = [
		AlgorithmInfo("Astar", "algorithm::Astar", "search/astar.hpp", False, False),
		AlgorithmInfo("HAstar", "algorithm::hastarv2::HAstar_StatsSimple", "search/hastar/v2/hastar.hpp", True, False),
		AlgorithmInfo("UGSA", "algorithm::ugsav5::UGSAv5_StatsSimple", "search/ugsa/v5/ugsa_v5.hpp", True, True),
		#AlgorithmInfo("Bugsy", "algorithm::Bugsy", "search/bugsy.hpp", False, True)
		AlgorithmInfo("Bugsy_Norm", "algorithm::Bugsy_Norm", "search/bugsy.hpp", False, True)

		]



DOMS =	[
		DomainInfo("tiles_8h_5", tiles_stack(3,3,False,True,5), "domain/tiles/fwd.hpp", True, "tiles_8"),
		DomainInfo("tiles_8hw_5", tiles_stack(3,3,True,True,5), "domain/tiles/fwd.hpp", True, "tiles_8"),
		#DomainInfo("tiles_15h_5", tiles_stack(4,4,False,True,7), "domain/tiles/fwd.hpp", True, "tiles_15"),
		#DomainInfo("tiles_15hw_5", tiles_stack(4,4,True,True,7), "domain/tiles/fwd.hpp", True, "tiles_15"),
		DomainInfo("pancake_10_7_2", pancake_stack_ignore(10, 7, 2), "domain/pancake/fwd.hpp", True, "pancake_10"),
		DomainInfo("pancake_10", pancake_stack_single(10, True), "domain/pancake/fwd.hpp", False, "pancake_10"),
		DomainInfo("gridnav_20", gridnav_blocked(20, 20, False, True, True), "domain/gridnav/fwd.hpp", False, "gridnav_20"),
		DomainInfo("gridnav_20", gridnav_blocked_stack_merge(20, 20, False, True, 3, 3, 2), "domain/gridnav/fwd.hpp", True, "gridnav_20"),
		]

PROBLEM_SETS =	[
		ProblemSetInfo("tiles_8", "tiles_8.json", gen_tiles_problems.genTilesProblemSet, (3, 3, 5)),
		ProblemSetInfo("tiles_15", "tiles_15.json", gen_tiles_problems.genTilesProblemSet, (4, 4, 2)),
		ProblemSetInfo("pancake_10", "pancake_10.json", gen_pancake_problems.genPancakeProblemSet, (10, 5)),
		ProblemSetInfo("gridnav_20", "gridnav_20.json", gen_gridnav_problems.genGridNavProblemSet, ("gridnav_20_map", 0.35, 20, 20, 5, 0.5))
				]

WEIGHTS = ((1,0),(1,0.01),(1,0.1),(1,1),(1,10),(1,100),(0,1))
NORM_WEIGHTS = ((1,1), (10, 1), (100, 1), (1000, 1), (1000000, 1))




def trial_A(usedalgdom = False):
	
	algs = [AlgorithmInfo.lookup["Astar"]]
	weights = NORM_WEIGHTS
	
	tiles_doms = [DomainInfo.lookup["tiles_8h_5"], DomainInfo.lookup["tiles_8wh_5"]]
	
	if usedalgdom:
		return [(a,d) for a in algs for d in tiles_doms]

	ps_t8 = ProblemSetInfo.lookup["tiles_8.json"]
	ps_pc10 = ProblemSetInfo.lookup["pancake_10.json"]
	ps_gn20 = ProblemSetInfo.lookup["gridnav_20.json"]
	
	ps_t8.load()
	ps_pc10.load()
	ps_gn20.load()
	
	exec_t8 = []
	
	execParamsLst = [ (d, a, w, p) for d in tiles_doms for a in algs for w in weights for p in ps_t8.problems ]
	
	for (d, a, w, p) in execParamsLst:
		key = (tile_doms.index(d), algs.index(a), weights.index(w), ps_t8.problems.index(p))
		exec_t8.append(ExecutionInfo(key, d, a, w, p)) 
	
	if not sys.argv.count("dump") > 0:
		executeParallelSearches(exec_t8)

	res_t8 = defaultdict(dict)
	
	for (d, a, w, p) in execParamsLst:
		key = (tile_doms.index(d), algs.index(a), weights.index(w), ps_t8.problems.index(p))
		ExecutionInfo.lookup[key].computeUtils()
		res_t8[d.name][a.name][str(w)][str(pi)]["params"] = ExecutionInfo.lookup[key].params
		res_t8[d.name][a.name][str(w)][str(pi)]["results"] = ExecutionInfo.lookup[key].results
	
	with open("trial_A_results.json", "w") as f:
		json.dump(res_t8, f)





if __name__ == "__main__":
	if len(sys.argv) == 1:
		print "gencode <outfile>"
		print "genprob [<probset> | all]"
		print "trial_A ['dump'] ..."
		#print "db <results> <dbtable>"

	if sys.argv[1] == "gencode":
		usedAlgDoms = trial_A(True)
		hdrs = []
		algdominfo = []
		
		for (a,d) in USED_ALGDOM:
			if hdrs.count(a.hdr) == 0:
				hdrs.append(a.hdr)
			
			if hdrs.count(d.hdr) == 0:
				hdrs.append(d.hdr)
			
			algdominfo.append((a.name + "_" + d.name, a.cls, d.cls))
		
		with open(sys.argv[2], "w") as f:
			f.write(gen_searcher.genSearcherCode(algdominfo, hdrs))		
		
	
	elif sys.argv[1] == "genprob":	
		ps = ProblemSetInfo.lookup[sys.argv[2]]
		ps.generate()
		ps.dump()


	elif sys.argv[1] == "trial_A":
		trial_A()



	"""
	elif sys.argv[1] == "db":
		db = MySQLdb.connect(host="localhost", user="matthew", passwd="debian", db="research")
		cur = db.cursor()
		
		tbname = sys.argv[3]
		
		with open(argv[2]) as f:
			resultSet = json.load(f)
		
		cur.execute("DROP TABLE IF EXISTS '{tb}'; CREATE TABLE '{tb}' (domain VARCHAR(20), algorithm VARCHAR(20), weight VARCHAR(20), problem INT, cost DOUBLE, length INT, baseExpd INT, allExpd INT, expTime DOUBLE, cpuTime DOUBLE, memory DOUBLE);".format(tb=tbname))
		
		for dk, dv in resultSet.iteritems():
			for ak, av in dv.iteritems():
				for wk, wv in av.iteritems():
					for pk, pv in wv.iteritems():
						
						insertStr = "INSERT INTO {tb} VALUES ('{d}','{a}','{w}','{p}', '{cst}', '{ln}', '{be}', '{ae}', '{et}', '{ct}', '{mem}');".format(
							tb=tbname, d=dk, a=ak, w=wk, p=pk, cst=pv["_sol_cost"], ln=pv["_sol_length"], be=pv["_base_expd"], ae=pv["_all_expd"], et=pv["_exptime"], ct=pv["_cputime"], mem=pv["_mem_used"])
						
						cur.execute(insertStr)
						
		db.commit()
		db.close()
	"""
		
		
