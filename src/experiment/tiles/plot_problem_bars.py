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


#(figure for each domain) * (subplot col for each val) * (subplot row for each weight) * (x tick for each algorithm) * (bar for each problem)
def plotA(resultSet, valsOfInterest):

	doms = resultSet["meta"]["domains"]
	algs = resultSet["meta"]["algorithms"]
	weights = resultSet["meta"]["weights"]
	nprobs = resultSet["meta"]["n_problems"]
	
	series = np.empty((len(doms), len(algs), len(weights), len(valsOfInterest), nprobs))

	for di in range(len(doms)):
		for ai in range(len(algs)):
			for wi in range(len(weights)):
				for vi in range(len(valsOfInterest)):
					for pi in range(nprobs):
						d = doms[di]
						a = algs[ai]
						w = weights[wi]
						v = valsOfInterest[vi]
						p = str(pi)
						
						if resultSet[d][a][w][p]["results"]["_result"] != "good":
							print d, a, w, v, resultSet[d][a][w][p]["results"]["_result"] 
							series[di][ai][wi][vi][pi] = 0
					
						else:
							series[di][ai][wi][vi][pi] = resultSet[d][a][w][p]["results"][valsOfInterest[vi]]
	
	
	for di in range(len(doms)): 
		
		fig, axs = plt.subplots(len(weights), len(valsOfInterest))
		fig.suptitle(resultSet["meta"]["domains"][di])
		fig.subplots_adjust(bottom=0.3, hspace=0.3)
		
		for wi in range(len(weights)):
			for vi in range(len(valsOfInterest)):
				
				if len(valsOfInterest) == 1:
					ax = axs[wi]
				else:
					ax = axs[wi][vi]
				
				ax.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='off')
				ax.get_yaxis().get_major_formatter().set_powerlimits((-1, 2))
				
				widthPerProb = 0.8 / nprobs
			
				for ai in range(len(algs)):
					ax.bar([x*widthPerProb + ai for x in range(nprobs)], series[di][ai][wi][vi], widthPerProb)


		for wi in range(len(weights)):
			if len(valsOfInterest) == 1:
				ax = axs[wi]
			else:
				ax = axs[wi][0]

			ax.set_ylabel(str(resultSet["meta"]["weights"][wi]), rotation=0, size="large", labelpad=50)
		

		for vi in range(len(valsOfInterest)):
			if len(valsOfInterest) == 1:
				ax = axs[0]
			else:
				ax = axs[0][vi]

			ax.set_title(valsOfInterest[vi])
		
		for vi in range(len(valsOfInterest)):
			if len(valsOfInterest) == 1:
				ax = axs[-1]
			else:
				ax = axs[-1][vi]
			
			ax.tick_params(axis='x', which='both', bottom='on', labelbottom='on')
			ax.set_xticks([(x+0.5) for x in range(len(algs))])
			ax.set_xticklabels(resultSet["meta"]["algorithms"], rotation="vertical")

	plt.show()



if __name__ == "__main__":
	if len(sys.argv) == 1:
		print "<resultfile> <valname> [<valname>...]"
		exit(0)
	
	with open(sys.argv[1]) as f:
		resultSet = json.load(f)

	
	valsOfInterest = sys.argv[2:]

	
	
	plotA(resultSet, valsOfInterest)
	
	#doProbPlot(sys.argv[1], sys.argv[2:], False)
