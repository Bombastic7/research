#!/bin/python

import sys
import os
import json
import numpy as np
import matplotlib.pyplot as plt


#(figure for each domain) * (subplot col for each val) * (subplot row for each weight) * (x tick for each algorithm) * (bar for each problem)
def plotA(resultSet, valsOfInterest, savePlots = False):

	doms = [i for i in resultSet.iterkeys()]
	algs = [i for i in resultSet[doms[0]].iterkeys()]
	weights = [i for i in resultSet[doms[0]][algs[0]].iterkeys()]
	nprobs = len([i for i in resultSet[doms[0]][algs[0]][weights[0]].iterkeys()])
	
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
		fig.suptitle(doms[di])
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

			ax.set_ylabel(str(weights[wi]), rotation=0, size="large", labelpad=50)
		

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
			ax.set_xticklabels(algs, rotation="vertical")





if __name__ == "__main__":
	if len(sys.argv) == 1:
		print "[-o] <resultfile> <valname0> [<valname1>...]"
		exit(0)
	
	resfile = sys.argv[1]
	doBigPlot = False
	
	if sys.argv[1] == "-o":
		resfile = sys.argv[2]
		doBigPlot = True
	
	
	with open(resfile) as f:
		resultSet = json.load(f)

	
	if doBigPlot:
		valsOfInterest = sys.argv[3:]
		
		doms = [i for i in resultSet.iterkeys()]
		
		for di in range(len(doms)):
			outdir = "plots/{0}/".format(doms[di])
			os.makedirs(outdir)
			
			for vi in range(len(valsOfInterest)):
				plotA(resultSet, [valsOfInterest[vi]], True)
				plt.savefig(outdir + valsOfInterest[vi] + ".png")
				plt.close()
		

	else:
		valsOfInterest = sys.argv[2:]
		plotA(resultSet, valsOfInterest)
		plt.show()
	
	#doProbPlot(sys.argv[1], sys.argv[2:], False)
