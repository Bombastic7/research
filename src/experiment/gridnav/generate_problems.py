 
def genGridNavMap(h, w, block, fname):
	
	with open(fname, "w") as f:
		for i in range(0, h*w):
			
			val = 0
			if random.uniform(0, 1) < block:
				val = 1
			
			f.write(str(val) + " ")
			
			if (i+1) % w == 0:
				f.write("\n")



def genGridNavProblemSet(fname, fmapname, h, w, nprobs, mindist):
	
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
		






def _bstr(b):
	return "true" if b else "false"


def gridnav_blocked(height, width, mv8, cstLC, hr):
	
	declStr = "gridnav::blocked::GridNav_DomainStack_single<{0}, {1}, {2}, {3}, {4}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), _bstr(hr))
	
	return declStr
	
	
	


def gridnav_blocked_stack_merge(height, width, mv8, cstLC, hfact, wfact, fillfact, maxAbtLvl = 1000):
	
	declStr = "gridnav::blocked::GridNav_DomainStack_MergeAbt<{0},{1},{2},{3},{mxL},{4},{5},{6}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), hfact, wfact, fillfact, mxL=maxAbtLvl)
	
	return declStr
