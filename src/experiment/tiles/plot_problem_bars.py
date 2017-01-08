#!/bin/python

import sys
import json
import numpy as np
import matplotlib.pyplot as plt

"""
def extractValuesOfInterest(resfile, valsOfInterest):
	
	with open(resfile) as f:
		resultSet = json.load(f)

	resDict = {}

	for di in range(len(DOMS)):
		d = DOMS[di]
		
		series = np.empty([len(WEIGHTS), len(valsOfInterest), len(ALGS), NPROBLEMS])
		
		for wi in range(len(WEIGHTS)):
			for vi in range(len(valsOfInterest)):
				
				for ai in range(len(ALGS)):
					
					for pi in range(NPROBLEMS):
						series[wi][vi][ai][pi] = resultSet[makeAlgDomName(ALGS[ai], d)][str(WEIGHTS[wi])][str(pi)][valsOfInterest[vi]]

		resDict[di] = series

	return resDict



def doProbPlot(resfile, valsOfInterest, avg=True):
	
	vals = extractValuesOfInterest(resfile, valsOfInterest)

	for di, series in vals.iteritems():
		d = DOMS[di]
		
		fig, axs = plt.subplots(len(WEIGHTS), len(valsOfInterest))
		#fig.tight_layout()
		fig.suptitle(d["name"])
		fig.subplots_adjust(bottom=0.3, hspace=0.3)
		
		for wi in range(len(WEIGHTS)):
			for vi in range(len(valsOfInterest)):
				ax = axs[wi][vi]
				
				ax.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='off')
				ax.get_yaxis().get_major_formatter().set_powerlimits((-1, 2))
				
				
				if avg:
					width = 0.8
					heights = [np.mean(series[wi,vi,ai,:]) for ai in range(len(ALGS))]
					errs = [np.std(series[wi,vi,ai,:], ddof=1) for ai in range(len(ALGS))]

					
					ax.bar(range(len(ALGS)), heights, width, yerr=errs)
					
				
				else:
					widthPerProb = 0.8 / NPROBLEMS
					
					for ai in range(len(ALGS)):
						ax.bar([x*widthPerProb + ai for x in range(NPROBLEMS)], series[wi][vi][ai], widthPerProb)
			
		
		for wi in range(len(WEIGHTS)):
			ax = axs[wi][0]
			ax.set_ylabel(str(WEIGHTS[wi]), rotation=0, size="large", labelpad=50)
		
		for vi in range(len(valsOfInterest)):
			ax = axs[0][vi]
			ax.set_title(valsOfInterest[vi])
		
		for vi in range(len(valsOfInterest)):
			ax = axs[len(WEIGHTS)-1][vi]
			ax.tick_params(axis='x', which='both', bottom='on', labelbottom='on')
			ax.set_xticks([(x+0.5) for x in range(len(ALGS))])
			ax.set_xticklabels([alg["name"] for alg in ALGS], rotation="vertical")
			
		
		#fig.show()
	
	plt.show()
"""


def plotA(series, metainfo):
	assert(len(series.shape) == 4)
	
	for di in range(series.shape[0]): #domain
		
		fig, axs = plt.subplots(series.shape[2], 1)
		fig.suptitle(metainfo["dim_names"][di])
		fig.subplots_adjust(bottom=0.3, hspace=0.3)
		
		for wi in range(series.shape[2]): #weight
		
			ax = axs[wi]
			ax.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='off')
			ax.get_yaxis().get_major_formatter().set_powerlimits((-1, 2))
			
			widthPerProb = 0.8 / NPROBLEMS
		
			for ai in range(series.shape[1]): #algorithm
				ax.bar([x*widthPerProb + ai for x in range(series.shape[3])], series[di][ai][wi], widthPerProb)

		for wi in range(series.shape[2]):
			ax = axs[wi]
			ax.set_ylabel(metainfo["key_names"][wi], rotation=0, size="large", labelpad=50)
		
			botsubplot = axs[-1]
			botsubplot.tick_params(axis='x', which='both', bottom='on', labelbottom='on')
			botsubplot.set_xticks([(x+0.5) for x in range(series.shape[1])])
			botsubplot.set_xticklabels(metainfo["key_names"][1], rotation="vertical")
	
	plt.show()



if __name__ == "__main__":
	if len(sys.argv) == 1:
		print "<resultfile> <valname> [<valname>...]"
		exit(0)
	
	with open(sys.argv[1]) as f:
		resultSet = json.load(f)
	
	assert(resultSet["meta"]["num_dims"] == 4)
	
	dimLengths = resultSet["meta"]["dim_length"]
	
	series = np.empty(dimLengths)
	
	for i in range(dimLengths[0]):
		for j in range(dimLengths[1]):
			for k in range(dimLengths[2]):
				for l in range(dimLengths[3]):
					key0 = resultSet["meta"]["key_names"][0][i]
					key1 = resultSet["meta"]["key_names"][1][j]
					key2 = resultSet["meta"]["key_names"][2][k]
					key3 = resultSet["meta"]["key_names"][3][l]
					
					series[i][j][k][l] = resultSet[key0][key1][key2][key3]["results"][sys.argv[2]]
	
	plotA(series, resultSet["meta"])
	
	#doProbPlot(sys.argv[1], sys.argv[2:], False)
