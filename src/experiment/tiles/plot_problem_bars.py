#!/bin/python

import sys
import json
import numpy as np
import matplotlib.pyplot as plt

from configuration import DOMS, ALGS, WEIGHTS, VALUESOFINTEREST, NPROBLEMS, makeAlgDomName


def doMain():
	
	with open(sys.argv[1]) as f:
		resultSet = json.load(f)

	for d in [DOMS[1]]:
		
		series = np.empty([len(WEIGHTS), len(VALUESOFINTEREST), len(ALGS), NPROBLEMS])
		
		
		for wi in range(len(WEIGHTS)):
			for vi in range(len(VALUESOFINTEREST)):
				
				for ai in range(len(ALGS)):
					
					for pi in range(NPROBLEMS):
						
						if VALUESOFINTEREST[vi] in resultSet[makeAlgDomName(ALGS[ai], d)][str(WEIGHTS[wi])][str(pi)]:
							series[wi][vi][ai][pi] = resultSet[makeAlgDomName(ALGS[ai], d)][str(WEIGHTS[wi])][str(pi)][VALUESOFINTEREST[vi]]
						else:
							resultSet[makeAlgDomName(ALGS[ai], d)][str(WEIGHTS[wi])][str(pi)][VALUESOFINTEREST[vi]] = 0
	
		#print series
		#print len(series)					
		
		fig, axs = plt.subplots(len(WEIGHTS), len(VALUESOFINTEREST))
		#fig.tight_layout()
		fig.suptitle(d["name"])
		fig.subplots_adjust(bottom=0.3, hspace=0.3)
		
		for wi in range(len(WEIGHTS)):
			for vi in range(len(VALUESOFINTEREST)):
				ax = axs[wi][vi]
				
				ax.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='off')
				ax.get_yaxis().get_major_formatter().set_powerlimits((-1, 2))
				
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

"""
		for wf,wt in WEIGHTS:
			for vn in VALUESOFINTEREST:
				
				series = np.empty([len(ALGS), NPROBLEMS])
				
				for ai in range(len(ALGS)):
					for pi in range(NPROBLEMS):

						if vn in resultSet[makeAlgDomName(ALGS[ai], d)][str((wf,wt))][str(pi)]:
							series[ai][pi] = resultSet[makeAlgDomName(ALGS[ai], d)][str((wf,wt))][str(pi)][vn]
						else:
							series[ai][pi] = 0


				width = 0.8 / len(ALGS)
				
				barPlots = []
				
				
				plt.subplot(
				
				for ai in range(len(ALGS)):
					rects = plt.bar([x*width + ai for x in range(NPROBLEMS)], series[ai], width)
				
				plt.set_xticks([x + 0.4 for x in range(len(ALGS))])
				plt.set_xticklabels([alg["name"] for alg in ALGS])
				plt.show()
"""

if __name__ == "__main__":
	doMain()
