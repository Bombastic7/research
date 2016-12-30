#!/bin/python

import json
import sys
from ast import literal_eval

import configuration


def doMain():
	assert(len(sys.argv) >= 3)
	
	with open(sys.argv[1]) as f:
		resultSet = json.load(f)
	
	
	analyseDict = {}
	

	
	
	for d in configuration.DOMS:
		
		analyseDict[d["name"]] = {}

		for w in configuration.WEIGHTS:

			analyseDict[d["name"]][str(w)] = {}
			
			for a in configuration.ALGS:
				
				if a["abt"] and not d["abt"]:
					continue
				
				for (pk, res) in resultSet[configuration.makeAlgDomName(a,d)][str(w)].iteritems():
					
			
					if pk not in analyseDict[d["name"]][str(w)]:
							analyseDict[d["name"]][str(w)][pk] =  {"util_real" : [], "util_gend" : [], "util_expd" : [], "solcost":[]}
					
					(wf, wt) = w
					
						
					if "_solution_cost" in res:
						util_real = wf * res["_solution_cost"] + wt * res["_cputime"]
						util_gend = wf * res["_solution_cost"] + wt * res["_algorithm_report"]["gend"]
						util_expd = wf * res["_solution_cost"] + wt * res["_algorithm_report"]["expd"]
					
					else:
						util_real = sys.maxint
						util_gend = sys.maxint
						util_expd = sys.maxint
					
					analyseDict[d["name"]][str(w)][pk]["util_real"].append((util_real, a["name"]))
					analyseDict[d["name"]][str(w)][pk]["util_gend"].append((util_gend, a["name"]))
					analyseDict[d["name"]][str(w)][pk]["util_expd"].append((util_expd, a["name"]))
					
					if "_solution_cost" in res:
						analyseDict[d["name"]][str(w)][pk]["solcost"].append((res["_solution_cost"], a["name"]))
					else:
						analyseDict[d["name"]][str(w)][pk]["solcost"].append((sys.maxint, a["name"]))

	
	
		for d in analyseDict.iterkeys():
			for wk in analyseDict[d].iterkeys():
				for pk in analyseDict[d][wk].iterkeys():
					
					analyseDict[d][wk][pk]["util_real"].sort()
					analyseDict[d][wk][pk]["util_gend"].sort()
					analyseDict[d][wk][pk]["util_expd"].sort()
					analyseDict[d][wk][pk]["solcost"].sort()

					
	with open(sys.argv[2], "w") as f:
		json.dump(analyseDict, f, indent=4, sort_keys=True)


if __name__ == "__main__":
	doMain()
