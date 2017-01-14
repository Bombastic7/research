 #!/bin/python
 
import random

def getRandomPerm(sz):
	perm = range(0, sz)
	
	random.shuffle(perm)
	random.shuffle(perm)
	random.shuffle(perm)

	return tuple(perm)



def getTilesParity(perm, w, h):
	inversions = 0
	for i in range(0, len(perm)):
		for j in range(i, len(perm)):
			if perm[i] > perm[j] and perm[j] != 0:
				inversions += 1

	return inversions % 2


def getTilesSolvability(perm, goal, w, h):
	
	if w % 2 == 1:
		return getTilesParity(perm, w, h) == getTilesParity(goal, w, h)
	
	permRow = (perm.index(0) / w)
	goalRow = (goal.index(0) / w)
	
	blankRowParity = abs(permRow - goalRow) % 2
	
	return getTilesParity(perm, w, h) == blankRowParity



def getTilesInitState(w, h, goal):
	perm = getRandomPerm(h*w)
	
	if getTilesSolvability(perm, goal, w, h):
		return perm
	
	return getTilesInitState(w, h, goal)





		
def genTilesProblemSet(w, h, nprob):
	goalState = tuple(range(0, h*w))

	return [{"init":getTilesInitState(w, h, goalState), "goal":list(goalState)} for n in range(nprob)]
