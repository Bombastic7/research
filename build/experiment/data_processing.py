#!/bin/python

import numpy as np
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
						
						d1[ak][wk][dk][pk]["utility"] = wft[0] * goal_g + wft[1] * cpu_time
						
						if exptime is not None:
							d1[ak][wk][dk][pk]["cpu_time_fixed"] = exptime * rawdata[ak]["nullweight"][dk][pk]["expd"]
							d1[ak][wk][dk][pk]["utility_fixed"] = wft[0] * goal_g + wft[1] * d1[ak][wk][dk][pk]["cpu_time_fixed"]
		
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
						
						d1[ak][wk][dk][pk]["utility"] = wft[0] * goal_g + wft[1] * cpu_time
						
						if exptime is not None:
							d1[ak][wk][dk][pk]["cpu_time_fixed"] = exptime * rawdata[ak][wk][dk][pk]["expd"]
							d1[ak][wk][dk][pk]["utility_fixed"] = wft[0] * goal_g + wft[1] * d1[ak][wk][dk][pk]["cpu_time_fixed"]

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




def reduce_data1(d1):
	d2 = {}
	
	for ak in d1.keys():
		d2[ak] = {}
		for wk in d1[ak].keys():
			d2[ak][wk] = {}
			for dk in d1[ak][wk].keys():
				d2[ak][wk][dk] = {}
				
				util_series = []
				expd_series = []
				goal_depth_series = []
				goal_g_series = []
				cputime_series = []
				
				util_fixed_series = []
				cputime_fixed_series = []
				
				
				for pk in d1[ak][wk][dk].keys():
					astar_utility = float(d1["astar"][wk][dk][pk]["utility"])
					astar_expd = float(d1["astar"][wk][dk][pk]["expd"])
					astar_goal_depth = float(d1["astar"][wk][dk][pk]["goal_depth"])
					astar_goal_g = float(d1["astar"][wk][dk][pk]["goal_g"])
					astar_cputime = float(d1["astar"][wk][dk][pk]["cpu_time"])
					
					if "utility_fixed" in d1["astar"][wk][dk][pk]:
						astar_utility_fixed = float(d1["astar"][wk][dk][pk]["utility_fixed"])
						astar_cputime_fixed = float(d1["astar"][wk][dk][pk]["cpu_time_fixed"])
					
					#~ util_series.append(d1[ak][wk][dk][pk]["utility"])
					#~ expd_series.append(d1[ak][wk][dk][pk]["expd"])
					#~ goal_depth_series.append(d1[ak][wk][dk][pk]["goal_depth"])
					#~ goal_g_series.append(d1[ak][wk][dk][pk]["goal_g"])
					#~ cputime_series.append(d1[ak][wk][dk][pk]["cpu_time"])
				
					util_series.append(float(d1[ak][wk][dk][pk]["utility"]) / astar_utility)
					expd_series.append(float(d1[ak][wk][dk][pk]["expd"]) / astar_expd)
					goal_depth_series.append(float(d1[ak][wk][dk][pk]["goal_depth"]) / astar_goal_depth)
					goal_g_series.append(float(d1[ak][wk][dk][pk]["goal_g"]) / astar_goal_g)
					cputime_series.append(float(d1[ak][wk][dk][pk]["cpu_time"]) / astar_cputime)
					
					if "utility_fixed" in d1["astar"][wk][dk][pk]:
						util_fixed_series.append(float(d1[ak][wk][dk][pk]["utility_fixed"]) / astar_utility_fixed)
						cputime_fixed_series.append(float(d1[ak][wk][dk][pk]["cpu_time_fixed"]) / astar_cputime_fixed)
				
				for k, s in (
					("utility", util_series), 
					("expd", expd_series), 
					("goal_depth", goal_depth_series), 
					("goal_g", goal_g_series),
					("cpu_time", cputime_series)):
					d2[ak][wk][dk][k] = {}
					d2[ak][wk][dk][k]["mean"] = np.mean(s)
					d2[ak][wk][dk][k]["std"] = np.std(s, ddof=1)
					d2[ak][wk][dk][k]["median"] = np.median(s)
				
				if "utility_fixed" in d1["astar"][wk][dk][pk]:
					for k, s in (("utility_fixed", util_fixed_series), ("cpu_time_fixed", cputime_fixed_series)):
						d2[ak][wk][dk][k] = {}
						d2[ak][wk][dk][k]["mean"] = np.mean(s)
						d2[ak][wk][dk][k]["std"] = np.std(s, ddof=1)
						d2[ak][wk][dk][k]["median"] = np.median(s)

	return d2
