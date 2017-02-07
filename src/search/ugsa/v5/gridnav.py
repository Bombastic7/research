#!/bin/python



CELL_OPEN = 0
CELL_BLOCKED = 1



class State:
	def __init__(self, idx):
		self.idx = idx

	def __eq__(self, o):
		return self.idx == o.idx

	def __hash__(self):
		return self.idx





def starAbt(grps, rad):
	abtgrps = [None] * len(grps)
	
	outdegqueue = [ (len(grps[i]), i) for i in range(grps) ]
	outdegqueue.sort(cmp=lambda x, y: x[1] < y[1] if x[0] == y[0] else x[0] > y[0])
	
	assignedcells = 0
	curgrp = 0
	singletons = []
	
	def assignAbtGrp(i, depth, g):
		if abtgrps[i] is not None or depth > rad:
			return
		
		abtgrps[i] = g
		assignedcells += 1
		for e in grps[i]:
			assignAbtGrp(e[0], depth+1, g)
	
	
	for g in outdegreequeue:
		if abtgrps[g[1]] is not None:
			continue
	
		assignedcells = 0
		assignAbtGrp(g[1], 0, curgrp)
		curgrp += 1
		if assignedcells == 1:
			singletons.append(g[1])
	
	for i in singletons:
		if len(grps[i]) > 0:
			abtgrps[i] = abtgrps[grps[i][0][0]]
	
	curgrp = 0
	relabel = {}
	
	for i in range(abtgrps):
		if abtgrps[i] not in relabel:
			relabel[abtgrps[i]] = curgrp
			curgrp += 1
		
		abtgrps[i] = relabel[abtgrps[i]]
	
	abtedges = {}
		
	for i in range(abtgrps):
		for e in grps[i]:
			if abtgrps[i] == e[0]:
				continue

			k = (abtgrps[i], e[0])
			if k not in abtedges or abtedges[k] > e[1]:
				abtedges[k] = e[1]

	abtedgeslst = [[]] * curgrp
	
	for ((s, d), c) in abtedges.iteritems():
		abtedgeslst[s].append((d, c))
	
	return tuple(abtedgeslst)



class StarAbtDomain:
	def __init__(self, parentdom, rad):
		if isinstance(parentdom, Domain):
			basegrps = [None] * parentdom.size
			curgrp = 0
			
			for i in range(parentdom.size):
				if parentdom.cells[i] == CELL_OPEN:
					basegrps[i] = curgrp
					curgrp += 1
			
			basegrpslst = [None] * curgrp
			
			for i in range(parentdom.size):
				if basegrps[i] is None:
					continue
				n = parentdom.expand(State(i))
				
				basegrpslst[basegrp[i]] = []
				
				for (d, c) in n:
					basegrpslst[basegrp[i]].append((basegrps[d], c))

			self.grps = starAbt(basegrpslst, rad)
	
		else:
			self.grps = starAbt(parentdom.grps, rad)
	
	
	def isTrivial(self):
		for i in self.grps:
			if len(i) > 0:
				return True
		return False
	
	



class Domain:
	def __init__(self, fmapname, s0, goal, height, width):
		self.fmapname = fmapname
		self.s0 = s0
		self.goal = goal
		self.height = height
		self.width = width
		self.size = height * width
		
		self.cells = [None] * (self.size)
		
		with open(fmapname) as f:
			s = f.read().split()
			
			for i in range(self.size):
				self.cells[i] = int(s[i])
				assert(self.cells[i] == CELL_OPEN or self.cells[i] == CELL_BLOCKED)

		assert(self.cells[s0] == CELL_OPEN)
		assert(self.cells[goal] == CELL_OPEN)


	def expand(self, s):
		adjcells = []
		if s.idx >= self.width:
			adjcells.append((State(s.idx - self.width), 1))
		
		if s.idx < (self.height - 1) * self.width:
			adjcells.append((State(s.idx + self.width), 1))

		if s.idx % self.width != 0:
			adjcells.append((State(s.idx - 1), 1))

		if (s.idx + 1) % self.width != 0:
			adjcells.append((State(s.idx + 1), 1))

		return [c for c in adjcells if self.cells[c[0].idx] == CELL_OPEN]


	def initState(self):
		return State(self.s0)

	
	def checkGoal(self, s):
		return s.idx == self.goal
	
	
	def hval(self, s):
		return 0


	def spawnAbtDomain(self):
		
