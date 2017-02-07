#!/bin/python

import sys
import os
import multiprocessing
import subprocess
import json
from Queue import Empty
import multiprocessing.managers
import sqlite3

import gen_searcher
import gen_tiles_problems
import gen_pancake_problems
import gen_gridnav_problems



def workerRoutine(execqueue, resultsqueue, msgqueue, workerid):
	msgpfx = str(workerid) + ": "
	
	def writemsg(msg):
		msgqueue.put(msgpfx + msg)
	
	while True:
		try:
			(jobkey, jobname, params) = execqueue.get(True, 3)
		except Empty:
			writemsg("finished")
			break

		try:
			writemsg("starting " + jobname)
			proc = subprocess.Popen(["./searcher", "-s", jobname], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
			searcherOut = proc.communicate(input=bytearray(params))[0]
			resultsqueue.put((jobkey, jobname, searcherOut))

		except Exception as e:
			errres = {"_result":"exception", "_error_what": e.__class__.__name__ + " " + str(e) }
			resultsqueue.put((jobkey, jobname, json.dumps(errres)))





def _bstr(b):
	return "true" if b else "false"


def tiles_stack(height, width, weighted, useH, Abt1Sz):
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, _bstr(weighted), _bstr(useH), Abt1Sz)
	return declStr


def pancake_stack_ignore(Ncakes, Abt1Sz, AbtStep, useH, useWeight):
	declStr = "pancake::Pancake_DomainStack_IgnoreAbt<{0},{1},{2},{3},{4}>".format(Ncakes, Abt1Sz, AbtStep, _bstr(useH), _bstr(useWeight))
	return declStr	

def gridnav_blocked_stack_merge(height, width, mv8, cstLC, useH, hfact, wfact, fillfact, maxAbtLvl = 1000):
	declStr = "gridnav::blocked::GridNav_DomainStack_MergeAbt<{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), _bstr(useH), maxAbtLvl, hfact, wfact, fillfact)
	return declStr

def gridnav_blocked_starabt(height, width, mv8, lc, useH):
	declStr = "gridnav::blocked::GridNav_DomainStack_StarAbt<{0}, {1}, {2}, {3}, {4}>"\
		.format(height, width, _bstr(mv8), _bstr(lc), _bstr(useH))
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
	
	def __init__(self, name, cls, hdr, isabt, probcls, conf = None):
		self.name = name
		self.cls = cls
		self.hdr = hdr
		self.isabt = isabt
		self.probcls = probcls
		self.conf = conf
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




ALGS = [
		AlgorithmInfo("Astar", "algorithm::Astar", "search/astar.hpp", False, False),
		AlgorithmInfo("HAstar", "algorithm::hastarv2::HAstar_StatsSimple", "search/hastar/v2/hastar.hpp", True, False),
		AlgorithmInfo("UGSA_Cost_Delay", "algorithm::ugsav5::UGSAv5_Cost_Delay", "search/ugsa/v5/ugsa_v5.hpp", True, True, {"resort":False}),
		AlgorithmInfo("UGSA_Cost_HBF", "algorithm::ugsav5::UGSAv5_Cost_HBF", "search/ugsa/v5/ugsa_v5.hpp", True, True, {"resort":False}),
		AlgorithmInfo("UGSA_Cost_CostOnly", "algorithm::ugsav5::UGSAv5_Cost_CostOnly", "search/ugsa/v5/ugsa_v5.hpp", True, True, {"resort":False}),
		#AlgorithmInfo("UGSA_Dist", "algorithm::ugsav5::UGSAv5_Dist", "search/ugsa/v5/ugsa_v5.hpp", True, True, {"resort":False}),
		#AlgorithmInfo("UGSA_CostOrDist", "algorithm::ugsav5::UGSAv5_CostOrDist", "search/ugsa/v5/ugsa_v5.hpp", True, True, {"resort":False}),
		#AlgorithmInfo("UGSA_CostAndDist", "algorithm::ugsav5::UGSAv5_CostAndDist", "search/ugsa/v5/ugsa_v5.hpp", True, True, {"resort":False}),
		AlgorithmInfo("Bugsy", "algorithm::Bugsy", "search/bugsy.hpp", False, True),
		AlgorithmInfo("Bugsy_Norm", "algorithm::Bugsy_Norm", "search/bugsy.hpp", False, True)

		]



DOMS =	[
		DomainInfo("tiles_8h_5", tiles_stack(3,3,False,True,5), "domain/tiles/fwd.hpp", True, "tiles_8"),
		DomainInfo("tiles_8hw_5", tiles_stack(3,3,True,True,5), "domain/tiles/fwd.hpp", True, "tiles_8"),
		DomainInfo("tiles_15h_8", tiles_stack(4,4,False,True,8), "domain/tiles/fwd.hpp", True, "tiles_15"),
		DomainInfo("tiles_15hw_8", tiles_stack(4,4,True,True,8), "domain/tiles/fwd.hpp", True, "tiles_15"),
		DomainInfo("pancake_10_7_2", pancake_stack_ignore(10, 7, 2, True, False), "domain/pancake/fwd.hpp", True, "pancake_10"),
		DomainInfo("gridnav_20", gridnav_blocked_starabt(20, 20, False, False, False), "domain/gridnav/fwd.hpp", True, "gridnav_20", {"radius":2}),
		DomainInfo("gridnav_5", gridnav_blocked_starabt(5, 5, False, True, True), "domain/gridnav/fwd.hpp", True, "gridnav_20"),
		]

PROBLEM_SETS =	[
		ProblemSetInfo("tiles_8", "tiles_8.json", gen_tiles_problems.genTilesProblemSet, (3, 3, 5)),
		ProblemSetInfo("tiles_15", "tiles_15.json", gen_tiles_problems.genTilesProblemSet, (4, 4, 5)),
		ProblemSetInfo("pancake_10", "pancake_10.json", gen_pancake_problems.genPancakeProblemSet, (10, 5)),
		ProblemSetInfo("gridnav_20_4way_2r", "gridnav_20_4way_2r.json", gen_gridnav_problems.genGridNavProblemSet, ("gridnav_20_map", 0.35, 20, 20, 5, 0.5, False, 2))
				]

WEIGHTS = ((1,0),(1,0.01),(1,0.1),(1,1),(1,10),(1,100),(0,1))
NORM_WEIGHTS = ((1,1), (10, 1), (100, 1), (1000, 1), (1000000, 1))





def trial_test_ugsa(appendAlgDoms = None):
	manager = multiprocessing.Manager()
	execqueue = manager.Queue()
	resultsqueue = manager.Queue()
	msgqueue = manager.Queue()
	
	#doms = [DomainInfo.lookup["tiles_8h_5"], DomainInfo.lookup["tiles_8hw_5"]]
	doms = [DomainInfo.lookup["gridnav_20"]]
	algs = []#[a for a in AlgorithmInfo.lookup.itervalues() if a.name.count("UGSA") > 0]
	algs.append(AlgorithmInfo.lookup["HAstar"])
	algs.append(AlgorithmInfo.lookup["Astar"])
	
	if appendAlgDoms is not None:
		appendAlgDoms.extend([(a, d) for a in algs for d in doms])
		return

	
	probset = ProblemSetInfo.lookup["gridnav_20_4way_2r.json"]
	probset.load()
	
	weights = [(1,1)]#, (10,1), (100,1), (1000,1)]
	problems = probset.problems
	
	searches = {}
	
	for d in doms:
		for a in algs:
			for w in weights:
				for p in problems:
					jobKey = (doms.index(d), algs.index(a), weights.index(w), problems.index(p))
					jobName = d.name + "_" + a.name + "_" + str(w).replace(" ", "") + "_" + str(problems.index(p))
					
					params = {}
					params["_name"] = a.name + "_" + d.name
					params["_domain"] = d.name
					params["_algorithm"] = a.name
					params["_wf"] = w[0]
					params["_wt"] = w[1]
					params["_problem"] = problems.index(p)
					params["_domain_conf"] = p
									
					if a.conf is not None:
						params["_algorithm_conf"] = a.conf.copy()
					else:
						params["_algorithm_conf"] = {}
					
					if d.conf is not None:
						for (k,v) in d.conf.iteritems():
							params["_domain_conf"][k] = v
					
					params["_algorithm_conf"]["wf"] = w[0]
					params["_algorithm_conf"]["wt"] = w[1]

					params["_time_limit"] = 300#TIME_LIMIT
					params["_memory_limit"] = 8000#WORKER_MEM

					searches[jobName] = {"params":params}
					execqueue.put((jobKey, jobName, json.dumps(params)))

	
	if sys.argv.count("dump") == 0:
		workers = [multiprocessing.Process(target=workerRoutine, args=(execqueue, resultsqueue, msgqueue, i)) for i in range(1)]
		
		for i in workers:
			i.start()
	
		while len(workers) > 0:
			while True:
				try:
					print msgqueue.get_nowait()
				except Empty:
					break
			
			for i in workers:
				i.join(1)
				if not i.is_alive():
					workers.remove(i)
		
		while True:
			try:
				print msgqueue.get_nowait()
			except Empty:
				break
		
		
		while True:
			try:
				(jobKey, jobName, res) = resultsqueue.get(True, 1)
				assert(jobName in searches)
				try:
					searches[jobName]["results"] = json.loads(res)
				except Exception:
					print "Bad json string", jobName, res
			except Empty:
				break

	with open("ugsa_test2.json", "w") as f:
		json.dump(searches, f, indent=4, sort_keys=True)



def extractToDB(jfile, dbfile):
	with open(jfile) as f:
		searches = json.load(f)
	
	conn = sqlite3.connect(dbfile)
	c = conn.cursor()
	
	c.execute("""DROP TABLE IF EXISTS results""")
			
	c.execute("""CREATE TABLE results (domain text, algorithm text, weights text, wf real, wt real, problem int, 
				result text, 
				sollength int,
				solcost real,
				mem real,
				walltime real,
				cputime real,
				base_expd int,
				all_expd int)""")

	for (k, v) in searches.iteritems():
		params = v["params"]
		res = v["results"]
		c.execute("""INSERT INTO results VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)""", (
			params["_domain"],
			params["_algorithm"],
			str(params["_wf"]) + "_" + str(params["_wt"]),
			params["_wf"],
			params["_wt"],
			params["_problem"],
			res["_result"],
			res["_sol_length"] if "_sol_length" in res else None,
			res["_sol_cost"] if "_sol_cost" in res else None,
			res["_mem_used"] if "_mem_used" in res else None,
			res["_walltime"] if "_walltime" in res else None,
			res["_cputime"] if "_cputime" in res else None,
			res["_base_expd"] if "_base_expd" in res else None,
			res["_all_expd"]  if "_all_expd" in res else None
			))
	
	conn.commit()
	conn.close()



if __name__ == "__main__":

	if sys.argv[1] == "gencode":
		usedAlgDoms = []
		trial_test_ugsa(usedAlgDoms)
		
		hdrs = []
		usedAlgDoms = list(set(usedAlgDoms))
		algdominfo = []
		
		for (a,d) in usedAlgDoms:
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


	elif sys.argv[1] == "trial_test_ugsa":
		trial_test_ugsa()
