#!/bin/python

import json
import sys



def doMain():
	assert(len(sys.argv) >= 3)
	
	with open(sys.argv[1]) as f:
		resultSet = json.load(f)
	
	
	analyseDict = {}
	
	for (adk, adv) in resultset.iteritems():
		for (wk, wv) in adv.iteritems():
			
			analyseDict[wk] = {}
						
			for (pk, res) in wv.iteritems():
				
				if pk not in analyseDict[wk]:
					analyseDict[wk][pk] = {"util_real" : [], "util_gend" : [], "util_expd" : []}
				
				wf = str(wk)[0]
				wt = str(wk)[1]
				
				util_real = wf * res["_solution_cost"] + wt * res["_cputime"]
				util_gend = wf * res["_solution_cost"] + wt * res["_algorithm_report"]["gend"]
				util_expd = wf * res["_solution_cost"] + wt * res["_algorithm_report"]["expd"]
				
				analyseDict[wk][pk]["util_real"].append((adk, util_real)
				analyseDict[wk][pk]["util_gend"].append((adk, util_gend)
				analyseDict[wk][pk]["util_expd"].append((adk, util_expd)
				

	
	
		for wk in analyseDict.iterkeys():
			for pk in analyseDict[wk].iterkeys():
				
				analyseDict[wk][pk]["util_real"].sort()
				analyseDict[wk][pk]["util_gend"].sort()
				analyseDict[wk][pk]["util_expd"].sort()
				

					
	with open(sys.argv[2], "w") as f:
		json.dump(analyseDict, f, indent=4, sort_keys=True)


if __name__ == "__main__":
	doMain()
