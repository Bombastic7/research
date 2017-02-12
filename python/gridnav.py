#!/bin/python

import random

CELL_OPEN = 0
CELL_BLOCKED = 1






def starAbt(grps, rad):
	abtgrps = [None] * len(grps)

	outdegqueue = [ (len(grps[i]), i) for i in range(len(grps)) ]
	outdegqueue.sort(cmp=lambda x, y: x[1] - y[1] if x[0] == y[0] else y[0] - x[0])

	curgrp = 0
	singletons = []
	
	def assignAbtGrp(i, depth, g, abtgrps):	
		if abtgrps[i] is not None or depth > rad:
			return 0

		abtgrps[i] = g
		assignedcells = 0
		for e in grps[i]:
			assignedcells += assignAbtGrp(e[0], depth+1, g, abtgrps)
		return assignedcells + 1
	
	
	for g in outdegqueue:
		if abtgrps[g[1]] is not None:
			continue
	
		assignedcells = assignAbtGrp(g[1], 0, curgrp, abtgrps)
		curgrp += 1
		if assignedcells == 1:
			singletons.append(g[1])
	
	for i in singletons:
		if len(grps[i]) > 0:
			abtgrps[i] = abtgrps[grps[i][0][0]]
			if grps[i][0][0] in singletons:
				singletons.remove(grps[i][0][0])
	
	curgrp = 0
	relabel = {}
	
	for i in range(len(abtgrps)):
		if abtgrps[i] not in relabel:
			relabel[abtgrps[i]] = curgrp
			curgrp += 1
		
		abtgrps[i] = relabel[abtgrps[i]]
	
	abtedges = {}
		
	for i in range(len(abtgrps)):
		for e in grps[i]:
			if abtgrps[i] == abtgrps[e[0]]:
				continue

			k = (abtgrps[i], abtgrps[e[0]])
			if k not in abtedges or abtedges[k] > e[1]:
				abtedges[k] = e[1]

	abtedgeslst = [[] for i in range(curgrp)]

	for ((s, d), c) in abtedges.iteritems():
		abtedgeslst[s].append((d, c))

	return tuple(abtedgeslst), tuple(abtgrps)



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
				n = parentdom.expand(i)
				
				basegrpslst[basegrps[i]] = []
				
				for (d, c) in n:
					basegrpslst[basegrps[i]].append((basegrps[d], c))

			self.basegrps = basegrps
			self.grps, self.trns = starAbt(basegrpslst, rad)
			self.expectsCellIndex = True
			self.baserepr = [None] * parentdom.size
			for i in range(len(self.baserepr)):
				if parentdom.cells[i] != CELL_BLOCKED:
					self.baserepr[i] = self.trns[basegrps[i]]
			self.baseinst = parentdom
			
		else:
			self.grps, self.trns = starAbt(parentdom.grps, rad)
			self.baserepr = [None] * parentdom.size
			for i in range(parentdom.size):
				if parentdom.baserepr[i] is not None:
					self.baserepr[i] = self.trns[parentdom.baserepr[i]]
		
			self.baseinst = parentdom.baseinst

		self.goal = self.abstractState(parentdom.goal)
		self.rad = rad
		self.width = parentdom.width
		self.size = parentdom.size
		
		if not self.isTrivial():
			self.abtdom = StarAbtDomain(self, rad)
		else:
			self.abtdom = None
	
	
	def drawCells(self):
		for ln in [self.baserepr[s:s+self.width] for s in range(0, self.size, self.width)]:
			for i in range(self.width):
				if ln[i] is None:
					ln[i] = " "
				else:
					ln[i] = chr(ord("a") + ln[i] % 26)
			print ''.join(ln)

	
	def isTrivial(self):
		for i in self.grps:
			if len(i) > 0:
				return False
		return True
	
	
	def abstractState(self, bs):
		if hasattr(self, "expectsCellIndex"):
			return self.trns[self.basegrps[bs]]
		else:
			return self.trns[bs]
	
	
	def expand(self, s):
		return self.grps[s]
	
	
	def checkGoal(self, s):
		return s == self.goal

	def spawnAbtDomain(self):
		return self.abtdom




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

		self.abtdom = StarAbtDomain(self, 2)


	def expand(self, s):
		adjcells = []
		if s >= self.width:
			adjcells.append((s- self.width, 1))
		
		if s< (self.height - 1) * self.width:
			adjcells.append((s + self.width, 1))

		if s % self.width != 0:
			adjcells.append((s - 1, 1))

		if (s + 1) % self.width != 0:
			adjcells.append((s + 1, 1))

		return [c for c in adjcells if self.cells[c[0]] == CELL_OPEN]


	def initState(self):
		return self.s0

	
	def randomInitState(self):
		goalAbt = self._abstractStateToTop(self.goal)
		while True:
			s = random.randint(0, self.size-1)
			if self.cells[s] == CELL_BLOCKED:
				continue
			if self._abstractStateToTop(s) == goalAbt:
				return s

		
	def checkGoal(self, s):
		return s == self.goal
	
	
	def hval(self, s):
		return 0


	def spawnAbtDomain(self):
		return self.abtdom


	def _abstractStateToTop(self, bs):
		s = bs
		ad = self.abtdom
		
		while ad is not None:
			s = ad.abstractState(s)
			ad = ad.abtdom

		return s


	def drawCells(self):
		for ln in [self.cells[s:s+self.width] for s in range(0, self.size, self.width)]:
			print ''.join([str(i) for i in ln])
