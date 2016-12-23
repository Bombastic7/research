#!/bin/python

import os
import random
import json
import csv
import math


def _getRandomPerm(sz):
	perm = range(0, sz)
	
	random.shuffle(perm)
	random.shuffle(perm)
	random.shuffle(perm)

	return tuple(perm)



def _getTilesParity(perm, w, h):
	inversions = 0
	for i in range(0, len(perm)):
		for j in range(i, len(perm)):
			if perm[i] > perm[j] and perm[j] != 0:
				inversions += 1

	return inversions % 2


def _getTilesSolvability(perm, goal, w, h):
	
	if w % 2 == 1:
		return _getTilesParity(perm, w, h) == _getTilesParity(goal, w, h)
	
	permRow = (perm.index(0) / w)
	goalRow = (goal.index(0) / w)
	
	blankRowParity = abs(permRow - goalRow) % 2
	
	return _getTilesParity(perm) == blankRowParity



def _getTilesInitState(w, h, goal):
	perm = _getRandomPerm(h*w)
	
	if _getTilesSolvability(perm, goal, w, h):
		return perm
	
	return _getTilesInitState(w, h, goal)





		
def _genTilesProblemSet(w, h, nprob, fname):
	probs = {}

	goalState = tuple(range(0, h*w))

	for i in range(0, nprob):
		initState = _getTilesInitState(w, h, goalState)

		probs[str(i)] = {"init" : initState, "goal" : goalState}

	
	with open(fname, "w") as f:
		json.dump(probs, f, indent=4, sort_keys=True)




def _genPancakeProblemSet(sz, nprob, fname):
	probs = {}
	
	for i in range(0, nprob):
		probs[str(i)] = {"init" : _getRandomPerm(sz)}
	
	
	with open(fname, "w") as f:
		json.dump(probs, f, indent=4, sort_keys=True)



def _genGridNavMap(h, w, block, fname):
	
	with open(fname, "w") as f:
		for i in range(0, h*w):
			
			val = 0
			if random.uniform(0, 1) < block:
				val = 1
			
			f.write(str(val) + " ")
			
			if (i+1) % w == 0:
				f.write("\n")



def _genGridNavProblemSet(fname, fmapname, h, w, nprobs, mindist):
	
	cellsRows = []
	with open(fmapname, "rb") as f:
		reader = csv.reader(f, delimiter=" ")
		
		for row in reader:
			cellsRows.append(row)

	
	griddiag = math.hypot(h-1, w-1)
	
	probs = {}
	
	def startGoalCond(startpos, endpos):
		if abs(math.hypot(*startpos) - math.hypot(*goalpos)) < griddiag * mindist:
			return False
		
		if cellsRows[startpos[1]][startpos[0]] == 1 or cellsRows[goalpos[1]][goalpos[0]] == 1:
			return False
		
		return True

	
	for i in range(0, nprobs):
		startpos = (0,0)
		goalpos = (0,0)

		while not startGoalCond(startpos, goalpos):
			startpos = (random.randint(0, w-1), random.randint(0, w-1))
			goalpos = (random.randint(0, w-1), random.randint(0, w-1))
		
		probs[str(i)] = {"init" : startpos, "goal" : goalpos, "map" : fmapname}
	
	
	with open(fname, "w") as f:
		json.dump(probs, f, indent=4, sort_keys=True)
		



def generateFiles(probFiles):
	
	for p in probFiles:
		
		if p["type"] == "gridnav":
			if p["gen"] == "map":
				_genGridNavMap(p["dim"][0], p["dim"][1], p["blockedprob"], p["fname"])
			
			elif p["gen"] == "problems":
				_genGridNavProblemSet(p["fname"], p["map"], p["dim"][0], p["dim"][1], p["num"], p["mindistance"])
		
		elif p["type"] == "pancake10":
				_genPancakeProblemSet(p["size"], p["num"], p["fname"])
		
		elif p["type"] == "tiles8":
				_genTilesProblemSet(3, 3, p["num"], p["fname"])

		else:
			pass
	
	

