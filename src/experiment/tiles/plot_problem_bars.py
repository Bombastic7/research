#!/bin/python

import sys
import json
import numpy as np
import matplotlib.pyplot as plt

from configuration import DOMS, ALGS, WEIGHTS, VALUESOFINTEREST, NPROBLEMS, makeAlgDomName


def doMain():
	
	with open(sys.argv[1]) as f:
		resultSet = json.load(f)

	for d in DOMS:
		for wf,wt in WEIGHTS:
			for vn in VALUESOFINTEREST:
				
				series = np.empty([len(ALGS), NPROBLEMS])
				
				for ai in range(len(ALGS)):
					for pi in range(NPROBLEMS):

						if vn in resultSet[makeAlgDomName(ALGS[ai], d)][str((wf,wt))][str(pi)]:
							series[ai][pi] = resultSet[makeAlgDomName(ALGS[ai], d)][str((wf,wt))][str(pi)][vn]
						else:
							series[ai][pi] = 0

				print series


if __name__ == "__main__":
	doMain()
