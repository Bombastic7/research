#!/bin/python

import os
import random
import json
import sys
import copy
import subprocess
import multiprocessing


RUNS_FILE = "run_set.json"

RESULTS_FILE = "results_set.json"


N_WORKERS = multiprocessing.cpu_count()






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






if __name__ == "__main__":
	
	
	with open(RUNS_FILE) as f:
		RUNS = json.load(f)
		RUN_KEYS = [ i for i in RUNS.iterkeys() ]
	
	workerPool = multiprocessing.Pool(N_WORKERS)
	
	RESULTS = []
	
	multiprocessing.Queue(len(

print "Using", N_WORKERS, "workers"

for (st, ed) in getChunks(len(RUN_KEYS), N_WORKERS*4):
	#print "Starting", (st, ed), " KEYS: ", RUN_KEYS[st], RUN_KEYS[ed-1]
	RESULTS.append(workerPool.map(execSearchRun, RUN_KEYS[st:ed]))
	print "Done", (st, ed), " KEYS: ", RUN_KEYS[st], RUN_KEYS[ed-1]


with open(RESULTS_FILE, "w") as f:
	
	resDict = {}
	
	for (key, res) in RESULTS:
		resDict[key] = res
	
	json.dump(resDict, f, indent=4, sort_keys=True)




