#!/bin/python


import json

import copy


def normalise_data0(rawdata, exptime = None):
	d1 = {}
	
	weights = {"1~1": (1,1), "1~1e3": (1,1e3), "1~1e6": (1,1e6)}
	
	for ak in rawdata.iterkeys():
		d1[ak] = {}
		
		if "nullweight" in rawdata[ak].keys():
			assert(len(rawdata[ak].keys()) == 1)
		
			for wk, wft in weights.iteritems():
				d1[ak][wk] = {}
				
				for dk in rawdata[ak]["nullweight"].keys():
					d1[ak][wk][dk] = {}
					
					for pk in rawdata[ak]["nullweight"][dk].keys():
						
						d1[ak][wk][dk][pk] = copy.deepcopy(rawdata[ak]["nullweight"][dk][pk])
						
						goal_g = rawdata[ak]["nullweight"][dk][pk]["goal_g"]
						cpu_time = rawdata[ak]["nullweight"][dk][pk]["cpu_time"]
						
						if exptime is None:
							d1[ak][wk][dk][pk]["utility"] = wft[0] * goal_g + wft[1] * cpu_time
						else:
							d1[ak][wk][dk][pk]["utility"] = wft[0] * goal_g + wft[1] * exptime * rawdata[ak]["nullweight"][dk][pk]["expd"]
		
		else:
			for wk in rawdata[ak].keys():
				d1[ak][wk] = {}
				
				assert(wk in weights)
				wft = weights[wk]
				
				for dk in rawdata[ak][wk].keys():
					d1[ak][wk][dk] = {}
					
					for pk in rawdata[ak][wk][dk].keys():
						d1[ak][wk][dk][pk] = copy.deepcopy(rawdata[ak][wk][dk][pk])
						
						goal_g = rawdata[ak][wk][dk][pk]["goal_g"]
						cpu_time = rawdata[ak][wk][dk][pk]["cpu_time"]
						
						if exptime is None:
							d1[ak][wk][dk][pk]["utility"] = wft[0] * goal_g + wft[1] * cpu_time
						else:
							d1[ak][wk][dk][pk]["utility"] = wft[0] * goal_g + wft[1] * exptime * rawdata[ak][wk][dk][pk]["expd"]

	return d1



def validate_data1(d1):
	weightkeys = d1.values()[0].keys()
	domkeys = d1.values()[0].values()[0].keys()
	probkeys = d1.values()[0].values()[0].values()[0].keys()
	
	for ak in d1.keys():
		assert(d1[ak].keys() == weightkeys)
		for wk in weightkeys:
			assert(d1[ak][wk].keys() == domkeys)
			for dk in domkeys:
				assert(d1[ak][wk][dk].keys() == probkeys)
