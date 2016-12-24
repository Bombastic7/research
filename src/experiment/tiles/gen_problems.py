import configuration


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
		return _getTilesParity(perm, w, h) == _getTilesParity(goal, w, h)
	
	permRow = (perm.index(0) / w)
	goalRow = (goal.index(0) / w)
	
	blankRowParity = abs(permRow - goalRow) % 2
	
	return _getTilesParity(perm) == blankRowParity



def getTilesInitState(w, h, goal):
	perm = _getRandomPerm(h*w)
	
	if _getTilesSolvability(perm, goal, w, h):
		return perm
	
	return _getTilesInitState(w, h, goal)





		
def genTilesProblemSet(w, h, nprob, fname):
	probs = {}

	goalState = tuple(range(0, h*w))

	for i in range(0, nprob):
		initState = _getTilesInitState(w, h, goalState)

		probs[str(i)] = {"init" : initState, "goal" : goalState}

	
	with open(fname, "w") as f:
		json.dump(probs, f, indent=4, sort_keys=True)
