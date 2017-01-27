#!/bin/python

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sqlite3


def extractValue_ap(indict, dom, weight, val):
	algs = [i for i in indict[dom].iterkeys()]
	
	probs = [i for i in indict[dom][algs[0]][weight].iterkeys()]
	
	valseriesdict = {}
	
	for a in algs:
		valseriesdict[a] = [None] * len(probs)
		
		for p in [int(p) for p in probs]:
			valseriesdict[a][p] = indict[dom][a][weight][str(p)]["results"][val]
	

	return pd.DataFrame(valseriesdict)


def extractValue_wp(indict, dom, alg, val):
	weights = [i for i in indict[dom][alg].iterkeys()]
	
	probs = [i for i in indict[dom][alg][weights[0]].iterkeys()]
	
	valseriesdict = {}
	
	for w in weights:
		valseriesdict[w] = [None] * len(probs)
		
		for p in [int(p) for p in probs]:
			valseriesdict[w][p] = indict[dom][alg][w][str(p)]["results"][val]
	

	return pd.DataFrame(valseriesdict)



def extractAllValues(indict, vals):
	
	results = {}

	for v in vals:
		results[v] = {}

		for d in indict.iterkeys():
			for a in indict[d].iterkeys():
				for w in indict[d][a].iterkeys():
					for p in indict[d][a][w].iterkeys():
						results[v][(d, a, w, p)] = indict[d][a][w][p]["results"][v]
	
	return results
					
					

def plotFromDB(dbfile):
	plotres = {}
	
	conn = sqlite3.connect(dbfile)
	c = conn.cursor()

	c.execute("""SELECT results.domain FROM results GROUP BY results.domain;""")
	doms = [i[0] for i in c.fetchall()]

	c.execute("""SELECT results.algorithm FROM results GROUP BY results.algorithm;""")
	algs = [i[0] for i in c.fetchall()]

	c.execute("""SELECT results.weights FROM results GROUP BY results.weights;""")
	weights = [i[0] for i in c.fetchall()]
	
	
	
	for d in doms:
		for w in weights:
			plotres[d + "_" + w] = {}
			
			for a in algs:
				c.execute("""SELECT results.sollength FROM results WHERE 
							results.domain == ? AND 
							results.algorithm == ? AND
							results.weights == ? ORDER BY results.problem;""", (d, a, w))
				
				plotres[d + "_" + w][a] = [i[0] for i in c.fetchall()]
		

	for (plotname, dw) in plotres.iteritems():
		df = pd.DataFrame(dw)
		df.plot.bar()
		plt.suptitle(plotname)
		plt.show()


if __name__ == "__main__":
	plotFromDB("ugsa_test2.db")
