#!/bin/python

import os
import random
import json
import math
import copy


from alg_dom_defs import *




def importProblemSet(fname):
	with open(fname) as f:
		s = json.load(f)
	
	return [ c for (n, c) in s.items()]
		




PROBLEM_SET_NAMES =	[
					("tiles8", "problems/tiles8.json"),
					("pancake10", "problems/pancake10.json"),
					("gn10_10", "problems/gridnav10_10_mapA_problems.json")
					]


PROBLEMS_SETS = dict([(name, importProblemSet(fname)) for (name, fname) in PROBLEM_SET_NAMES])


WEIGHTS = [0] #, 0.001, 0.01, 0.1, 1, 10, 100, 1000]



def goodPair(dom, alg):
	return (not(alg[1]) or dom[2]) and (not(alg[2]) or dom[3])


def prepRuns():

	domalgpairs = [(dom[0], alg[0], dom[1]) for dom in DOMS for alg in ALGS if goodPair(dom, alg)]

	withWeights = [(dom, alg, w, conf) for (dom, alg, confname) in domalgpairs for w in WEIGHTS for conf in PROBLEMS_SETS[confname]]
	
	runsDict = {}
	
	for i in range(0, len(withWeights)):
		
		vals = withWeights[i]
		
		dt = {	"domain" : vals[0], 
				"algorithm" : vals[1], 
				"domain conf" : vals[3], 
				"wf" : 1, 
				"wt" : vals[2], 
				"algorithm conf" : {"wf" : 1, "wt" : vals[2]},
				"time limit" : 60,
				"memory limit" : 2000
			}
		
		runsDict[str(i)] = dt
	
	
	with open("run_set.json", "w") as f:
		json.dump(runsDict, f, indent=4, sort_keys=True)

	print len(runsDict), "runs generated"

	

prepRuns()


