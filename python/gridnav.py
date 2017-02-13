#!/bin/python

import random
import cPickle

CELL_OPEN = 0
CELL_BLOCKED = 1





class Domain(object):
	def __init__(self, cellmap, goal):
		self.cellmap = cellmap
		self.goal = goal
		self.height = cellmap.height
		self.width = cellmap.width
		self.size = self.height * self.width
		self.abtstack = None

		assert(self.cellmap.cells[goal] == CELL_OPEN)
	
	
	def expand(self, s):
		return [(c, 1) for c in self.cellmap.adjcells(s)]


	def checkGoal(self, s):
		return s == self.goal
	
	def hval(self, s):
		return 0

	def randomInitState(self):
		while True:
			c = random.randint(0, self.size-1)
			if self.cellmap.cells[c] == CELL_OPEN and self.abtstack._baseIdxConnected(c, self.goal):
				return c


	def drawCells(self):
		for ln in [self.cells[s:s+self.width] for s in range(0, self.size, self.width)]:
			print ''.join([str(i) for i in ln])



class StarAbtDomain(object):
	def _makeAbtGroups(baseGroups, rad):
		trnsFromBaseGroup = [None] * len(baseGroups)

		outdegqueue = [ (len(baseGroups[i]), i) for i in range(len(baseGroups)) ]
		outdegqueue.sort(cmp=lambda x, y: x[1] - y[1] if x[0] == y[0] else y[0] - x[0])

		curgrp = 0
		singletons = []
		
		def recAssignAbtGrp(i, depth, g, abtgrps):	
			if abtgrps[i] is not None or depth > rad:
				return 0

			abtgrps[i] = g
			assignedcells = 0
			for e in grps[i]:
				assignedcells += recAssignAbtGrp(e[0], depth+1, g, abtgrps)
			return assignedcells + 1
		
		
		for g in outdegqueue:
			if abtgrps[g[1]] is not None:
				continue
		
			assignedcells = recAssignAbtGrp(g[1], 0, curgrp, trnsFromBaseGroup)
			curgrp += 1
			if assignedcells == 1:
				singletons.append(g[1])
		
		for i in singletons:
			if len(baseGroups[i]) > 0:
				trnsFromBaseGroup[i] = trnsFromBaseGroup[baseGroups[i][0][0]]
				if baseGroups[i][0][0] in singletons:
					singletons.remove(baseGroups[i][0][0])
		
		curgrp = 0
		relabel = {}
		
		for i in range(len(trnsFromBaseGroup)):
			if trnsFromBaseGroup[i] not in relabel:
				relabel[trnsFromBaseGroup[i]] = curgrp
				curgrp += 1
			
			trnsFromBaseGroup[i] = relabel[trnsFromBaseGroup[i]]
		
		groupEdgesDict = {}
			
		for i in range(len(baseGroups)):
			for e in baseGroups[i]:
				if trnsFromBaseGroup[i] == trnsFromBaseGroup[e[0]]:
					continue

				k = (trnsFromBaseGroup[i], trnsFromBaseGroup[e[0]])
				if k not in groupEdgesDict or groupEdgesDict[k] > e[1]:
					groupEdgesDict[k] = e[1]

		groupEdges = [[] for i in range(curgrp)]

		for ((src, dst), cost) in groupEdgesDict.iteritems():
			groupEdges[src].append((dst, cost))

		return groupEdges, trnsFromBaseGroup



		
		
	def __init__(self, parentdom, abtRadius):
		self.abtRadius = abtRadius
		if isinstance(parentdom, Domain):
			_prepFromBase(parentdom)
			self.parentDomIsBase = True
		else:
			_prepFromAbt(parentdom)
			self.parentDomIsBase = False

		self.goal = self.abstractState(parentdom.goal)

	
	def _prepFromBase(self, parentdom):
		curgrp = 0
		trnsFromBaseIdx = [None] * parentdom.size
		
		for i in range(parentdom.size):
			if parentdom.cellmap[i] != CELL_BLOCKED:
				trnsFromBaseIdx[i] = curgrp
				curgrp += 1
				
		baseGroupEdges = [None] * curgrp
		
		for i in range(parentdom.size):
			if parentdom.cellmap[i] != CELL_BLOCKED:
				baseGroupEdges[trnsFromBaseIdx[i]] = [(trnsFromBaseIdx[j], cost) for (j, cost) in parentdom.expand(i)]

		self.trnsFromBaseIdx = trnsFromBaseIdx
		self.groupEdges, self.trnsFromBaseGroup = StarAbtDomain._makeAbtGroups(baseGroupEdges, self.abtRadius)

	
	def _prepFromAbt(self, parentdom):
		self.groupEdges, self.trnsFromBaseGroup = StarAbtDomain._makeAbtGroups(parentdom.groupEdges, self.abtRadius)

	
	def abstractState(self, bs):
		if self.parentDomIsBase:
			return self.trnsFromBaseGroup[self.trnsFromBaseIdx[bs]]
		return self.trnsFromBaseGroup[bs]

	def expand(self, s):
		return self.groupEdges[s]
	
	def checkGoal(self, s):
		return self.goal == s

	def isTrivial(self):
		for i in self.groupEdges:
			if len(i) > 0:
				return False
		return True



class StarAbtDomainStack(object):
	def __init__(self, basedom, abtRadius):
		self.doms = [basedom]
		self.abtRadius = abtRadius
		self._prepAbtDoms()
		self.doms[0].abtstack = self


	def _prepAbtDoms(self):
		self.doms.append(StarAbtDomain(self.doms[0], self.abtRadius))
		while not self.doms[-1].isTrivial():
			self.doms.append(StarAbtDomain(self.doms[-1], self.abtRadius))
	
	
	def getDomain(self, lvl):
		return self.doms[lvl]

	def _baseIdxConnected(self, a, b):
		a_abt = a
		b_abt = b
		
		for dom in self.doms[1:]:
			a_abt = dom.abstractState(a_abt)
			b_abt = dom.abstractState(b_abt)

		return a_abt == b_abt

	



class CellMap:
	#geninfo is (height, width, blockedprob)
	def __init__(self, mapname = None, geninfo = None):
		if mapname is not None:
			self._loadFile(mapname)
		elif geninfo is not None:
			self._genRandom(geninfo)
		else:
			raise ValueError
	
	
	def _genRandom(self, geninfo):
		height, width, blockedprob = geninfo
		assert(height * width > 0)
		assert(blockedprob >= 0 and blockedprob <= 1)
		
		cells = [None] * (height*width)
		for i in range(height * width):
			if random.random() < blockedprob:
				cells[i] = CELL_BLOCKED
			else:
				cells[i] = CELL_OPEN
		
		self.cells = cells
		self.height = height
		self.width = width
		self.blockedprob = blockedprob
	
	
	def _loadFile(self, mapname):
		with open(mapname + ".pk2") as f:
			self.height, self.width, self.blockedprob, self.cells = cPickle.load(f)

	
	def dump(self, mapname):
		mapinfo = (self.height, self.width, self.blockedprob, self.cells)
		with open(mapname + ".pk2", "w") as f:
			cPickle.dump(mapinfo, f, 2)

	
	def getAdj(self, s):
		adjcells = []
		if s >= self.width:
			adjcells.append(s - self.width)
		if s< (self.height - 1) * self.width:
			adjcells.append(s + self.width)
		if s % self.width != 0:
			adjcells.append(s - 1)
		if (s + 1) % self.width != 0:
			adjcells.append(s + 1)
		return [c for c in adjcells if self.cells[c] == CELL_OPEN]


