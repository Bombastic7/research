#!/bin/python

import os
import random
import json
import sys
import copy
import subprocess
from multiprocessing import Pool, cpu_count


RUNS_FILE = "run_set.json"

RESULTS_FILE = "results_set.json"


N_WORKERS = cpu_count()






def execSearchRun(key):
	ret = None
	try:
		ret = (key, json.loads(str(subprocess.check_output(["./searcher", RUNS_FILE, key]))))
	except subprocess.CalledProcessError as e:
		if e.returncode == 10:
			ret = (key, {"result" : "time limit"})
		elif e.returncode == 11:
			ret = (key, {"result" : "memory limit"})
		else:
			print "Unknown return code. Failed key:", key
			ret = (key, {"result" : "error", "error what" : str(e.returncode)})

	except:
		print "Unhandled exception. Failed key:", key
		ret = (key, { "result" : "error", "error what" : sys.exc_info()[0]})
	
	return ret
	

def getChunks(listsz, chunksz):
	chunks = []
	chunkstart = 0
	
	while chunkstart < listsz:
		top = min(chunkstart + chunksz, listsz)
		chunks.append((chunkstart, top))
		chunkstart = top
	
	return chunks



with open(RUNS_FILE) as f:
	global RUN_KEYS
	global RUNS
	
	RUNS = json.load(f)
	RUN_KEYS = [ i for i in RUNS.iterkeys() ]



workerPool = Pool(N_WORKERS)



RESULTS = []

for (st, ed) in getChunks(len(RUN_KEYS), 1):
	print "Starting", (st, ed), " KEYS: ", RUN_KEYS[st], RUN_KEYS[ed-1]
	RESULTS.append(workerPool.map(execSearchRun, RUN_KEYS[st:ed]))
	print "Done", (st, ed), " KEYS: ", RUN_KEYS[st], RUN_KEYS[ed-1]


with open(RESULTS_FILE, "w") as f:
	
	resDict = {}
	
	for (key, res) in RESULTS:
		resDict[key] = res
	
	json.dump(resDict, f, indent=4, sort_keys=True)




