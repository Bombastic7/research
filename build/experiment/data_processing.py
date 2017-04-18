#!/bin/python


import json




def normalise_data0(rawdata):
	d1 = {}
	
	weights = {"1~1": (1,1), "1~1e3": (1,1e3), "1~1e6": (1,1e6)}
	
	for ak in rawdata.iterkeys():
		if "nullweight" in rawdata[ak].keys():
			assert(len(rawdata[ak].keys()) == 1)
		
			for wk, wft in weights.iteritems():
				for dk in rawdata[ak]["nullweight"].keys():
					for pk in rawdata[ak]["nullweight"][dk].keys():
						
						d1[ak][wk][dk][pk] = rawdata[ak]["nullweight"][dk][pk]
						
						goal_g = rawdata[ak]["nullweight"][dk][pk]["goal_g"]
						cpu_time = rawdata[ak]["nullweight"][dk][pk]["cpu_time"]
						
						d1[ak][wk][dk][pk]["utility"] = wft[0] * goal_g + wft[1] * cpu_time
		
		else:
			for wk in rawdata[ak],keys():
				assert(wk in weights)
				wft = weights[wk]
				
				for dk in rawdata[ak][wk].keys():
					for pk in rawdata[ak][wk][dk].keys():
						d1[ak][wk][dk][pk] = rawdata[ak][wk][dk][pk]





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
