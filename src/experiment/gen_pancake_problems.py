import sys
import random
import json

def _getRandomPerm(sz):
	perm = range(0, sz)
	
	random.shuffle(perm)
	random.shuffle(perm)
	random.shuffle(perm)

	return tuple(perm)




def genPancakeProblemSet(sz, nprob):
	probs = [ _getRandomPerm(sz) for i in range(nprob) ]
	return probs



