#!/bin/python

import sys
import json
import numpy as np
import matplotlib.pyplot as plt

from configuration import DOMS, ALGS, WEIGHTS, VALUESOFINTEREST, NPROBLEMS, makeAlgDomName



def extractValuesOfInterest():
	
	with open(sys.argv[1]) as f:
		resultSet = json.load(f)

	resDict = {}

	for di in range(len(DOMS)):
		d = DOMS[di]
		
		series = np.empty([len(WEIGHTS), len(VALUESOFINTEREST), len(ALGS), NPROBLEMS])
		
		for wi in range(len(WEIGHTS)):
			for vi in range(len(VALUESOFINTEREST)):
				
				for ai in range(len(ALGS)):
					
					for pi in range(NPROBLEMS):
						
						if VALUESOFINTEREST[vi] in resultSet[makeAlgDomName(ALGS[ai], d)][str(WEIGHTS[wi])][str(pi)]:
							series[wi][vi][ai][pi] = resultSet[makeAlgDomName(ALGS[ai], d)][str(WEIGHTS[wi])][str(pi)][VALUESOFINTEREST[vi]]
						else:
							resultSet[makeAlgDomName(ALGS[ai], d)][str(WEIGHTS[wi])][str(pi)][VALUESOFINTEREST[vi]] = 0
		
		resDict[di] = series

	return resDict


	
def doProbPlot(avg=True):
	
	vals = extractValuesOfInterest()

	for di, series in vals.iteritems():
		d = DOMS[di]
		
		fig, axs = plt.subplots(len(WEIGHTS), len(VALUESOFINTEREST))
		#fig.tight_layout()
		fig.suptitle(d["name"])
		fig.subplots_adjust(bottom=0.3, hspace=0.3)
		
		for wi in range(len(WEIGHTS)):
			for vi in range(len(VALUESOFINTEREST)):
				ax = axs[wi][vi]
				
				ax.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='off')
				ax.get_yaxis().get_major_formatter().set_powerlimits((-1, 2))
				
				
				if avg:
					width = 0.8
					heights = [np.mean(series[wi,vi,ai,:]) for ai in range(len(ALGS))]
					errs = [np.std(series[wi,vi,ai,:], ddof=1) for ai in range(len(ALGS))]

					print series[wi,vi,ai,:], d["name"], WEIGHTS[wi], ALGS[ai]["name"]
					
					ax.bar(range(len(ALGS)), heights, width, yerr=errs)
					
				
				else:
					widthPerProb = 0.8 / NPROBLEMS
					
					for ai in range(len(ALGS)):
						ax.bar([x*widthPerProb + ai for x in range(NPROBLEMS)], series[wi][vi][ai], widthPerProb)
			
		
		for wi in range(len(WEIGHTS)):
			ax = axs[wi][0]
			ax.set_ylabel(str(WEIGHTS[wi]), rotation=0, size="large", labelpad=50)
		
		for vi in range(len(VALUESOFINTEREST)):
			ax = axs[0][vi]
			ax.set_title(VALUESOFINTEREST[vi])
		
		for vi in range(len(VALUESOFINTEREST)):
			ax = axs[len(WEIGHTS)-1][vi]
			ax.tick_params(axis='x', which='both', bottom='on', labelbottom='on')
			ax.set_xticks([(x+0.5) for x in range(len(ALGS))])
			ax.set_xticklabels([alg["name"] for alg in ALGS], rotation="vertical")
			
		
		#fig.show()
	
	plt.show()



if __name__ == "__main__":
	doProbPlot(True)
