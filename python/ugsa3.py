#!/bin/python

import math
import hastar
import gridnav

from functools import total_ordering
from nodeheap import NodeHeap



@total_ordering
class BaseNode:
	def __init__(self, s, depth, g, u, parent):
		self.s = s
		self.g = g
		self.depth = depth
		self.u = u
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s

	def __lt__(self, o):
		if self.u != o.u:
			return self.u < o.u
		if self.depth != o.depth:
			return self.depth > o.depth
		return self.g > o.g
		

	def __hash__(self):
		return hash(self.s)



@total_ordering
class AbtNode:
	def __init__(self, s, depth, g, parent):
		self.s = s
		self.g = g
		self.depth = depth
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s

	def __lt__(self, o):
		if self.uf != o.uf:
			return self.uf < o.uf
		if self.ug != o.ug:
			return self.uf > o.uf
		if self.depth != o.depth:
			return self.depth > o.depth
		return self.g > o.g
		

	def __hash__(self):
		return hash(self.s)

	def __str__(self):
		st = "[" + str(self.s) + " " + str(self.depth) + " " + str(self.g)
		if hasattr(self, "ug"):
			st += " " + str(self.ug) + " " + str(self.uf)
		st += "]"
		return st 



class BestFirstUtilSearch:
	def __init__(self, wf, wt, domstack, lvl):
		self.wf = wf
		self.wt = wt
		self.domstack = domstack
		self.dom = domstack.getDomain(lvl)
		self.lvl = lvl
		self.powsumcache = {}

	def _powsum(self, n):
		if n not in self.powsumcache:
			acc = 0
			for i in range(1, n+1):
				acc += math.pow(self.bf, i)
			self.powsumcache[n] = acc
		return self.powsumcache[n]


	def setParams(self, k, bf):
		self.k = k
		self.bf = bf
		self.powsumcache = {}
		
	def _compUtil(self, g, depth):
		return self.wf*g + self.wt*self.k*self._powsum(depth)


	def doSearch(self, s0, initdepth):
		self.solpath = []
		self.openlist = NodeHeap()
		self.closedlist = {}

		n0 = AbtNode(s0, initdepth, 0, None)
		n0.ug = n0.uf = self._compUtil(n0.g, n0.depth)
		n0.isopen = True
		
		self.openlist.push(n0)
		self.closedlist[s0] = n0

		while True:
			n = self.openlist.pop()
			n.isopen = False

			if self.dom.checkGoal(n.s):
				break

			for c, edgecost in self.dom.expand(n.s):
				if n.parent is not None and c == n.parent.s:
					continue

				cg = n.g+edgecost
				cdepth = n.depth+1
				cn = AbtNode(c, cdepth, cg, n)
				cn.ug = cn.uf = self._compUtil(cg, cdepth)
				cn.isopen = True

				if c not in self.closedlist:
					self.openlist.push(cn)
					self.closedlist[c] = cn
					
				else:
					dup = self.closedlist[c]
					
					if cn.ug < dup.ug:
						dupstr = str(dup)
						dup.g = cn.g
						dup.depth = cn.depth
						dup.ug = cn.ug
						dup.uf = cn.uf
						dup.parent = n

						if dup.isopen:
							self.openlist.update(dup.openidx)
							
						else:
							self.openlist.push(dup)
							dup.isopen = True

		self.solpath = [n]
		while self.solpath[0].parent is not None:
			self.solpath.insert(0, self.solpath[0].parent)

		assert(n.ug == n.uf)
		return n.uf
		
	
	
	



class UGSA2_AbtSearch:
	def __init__(self, wf, wt, domstack, lvl, useCaching = True):
		self.wf = wf
		self.wt = wt
		self.domstack = domstack
		self.dom = domstack.getDomain(lvl)
		self.lvl = lvl
		self.useCaching = useCaching
		self.powsumcache = {}
		self.cache_exact = {}
		self.cache_partial = {}
		
		if useCaching and domstack.getTopLevel() > lvl:
			self.abtSearch = UGSA2_AbtSearch(wf, wt, domstack, lvl+1, False)
		else:
			self.abtSearch = None

		self.test_uncachedAlg = BestFirstUtilSearch(wf, wt, domstack, lvl)

		self.test_best_h = {}

	
	def setParams(self, k, bf):
		self.k = k
		self.bf = bf
		self.powsumcache = {}
		self.cache_exact = {}
		self.cache_partial = {}
		
		if self.abtSearch is not None:
			self.abtSearch.setParams(k, bf)

		self.test_uncachedAlg.setParams(k, bf)
		

	def _remExp(self, depth):
		return self.k*self._powsum(depth)

	def _compUtil(self, g, depth):
		return self.wf*g + self.wt*self._remExp(depth)

	def _powsum(self, n):
		if n not in self.powsumcache:
			acc = 0
			for i in range(1, n+1):
				acc += math.pow(self.bf, i)
			self.powsumcache[n] = acc
		return self.powsumcache[n]


	def _evalu(self, n):
		n.ug = self._compUtil(n.g, n.depth)
		n.uf = n.ug
		n.isExact = False

		if not self.useCaching:
			return

		if n.s in self.cache_exact:
			for goalState, ent in self.cache_exact[n.s].iteritems():
				if ent[0] <= n.depth and ent[1] >= n.depth:
					n.goal_g = n.g + ent[2]
					n.goal_depth = n.depth + ent[3]
					n.uf = self._compUtil(n.goal_g, n.goal_depth)
					n.isExact = True
					n.inferredGoalState = goalState
					return

	
		if n.s not in self.cache_partial:
			self.cache_partial[n.s] = {}

		if n.depth not in self.cache_partial[n.s]:
			#Use prev encounters at lower depths or do abt search starting at current depth of this node?
			uh1 = 0
			"""
			for cachedDepth, cachedU in self.cache_partial[n.s].iteritems():
				if cachedDepth > n.depth:
					continue
				if cachedU > uh1:
					uh1 = cachedU
			"""
			if self.abtSearch is not None:
				abtstate = self.domstack.getDomain(self.lvl+1).abstractState(n.s)
				uh2 = self.abtSearch.doSearch(abtstate)
			else:
				uh2 = 0

			self.cache_partial[n.s][n.depth] = max(uh1, uh2)

			#assert(max(uh1, uh2) <= self.test_uncachedAlg.doSearch(n.s, n.depth))
			"""
			if uh1 > uh2:
				self.test_best_h[n.s] = "lowerDepth"
			elif uh1 < uh2:
				self.test_best_h[n.s] = "abt"
			else:
				self.test_best_h[n.s] = "eq"
			"""
		n.uf = n.ug + self.cache_partial[n.s][n.depth]



	def _doCaching(self):
		if self.solpath[-1].isExact:
			goalState = self.solpath[-1].inferredGoalState
			goal_g = self.solpath[-1].goal_g
			goal_depth = self.solpath[-1].goal_depth
		else:
			goalState = self.solpath[-1].s
			goal_g = self.solpath[-1].g
			goal_depth = self.solpath[-1].depth

		goal_ug = self._compUtil(goal_g, goal_depth)
		
		for n in self.closedlist.itervalues():
			if n.isopen:
				continue

			pu = goal_ug - n.ug

			if n.depth not in self.cache_partial[n.s]: #self.cache_partial[n.s] would be created in _evalu
				self.cache_partial[n.s][n.depth] = 0
			
			if self.cache_partial[n.s][n.depth] < pu:
				self.cache_partial[n.s][n.depth] = pu

		for n in self.solpath:
			if n.isExact:
				if n is self.solpath[-1]:
					continue
				else:
					raise ValueError

			h = goal_g - n.g
			d = goal_depth - n.depth

			if n.s not in self.cache_exact:
				self.cache_exact[n.s] = {}

			if goalState not in self.cache_exact[n.s]:
				self.cache_exact[n.s][goalState] = [n.depth, n.depth, h, d]
			else:
				if n.depth < self.cache_exact[n.s][goalState][0]:
					self.cache_exact[n.s][goalState][0] = n.depth
				elif n.depth > self.cache_exact[n.s][goalState][1]:
					self.cache_exact[n.s][goalState][1] = n.depth


	def doSearch(self, s0):
		self.s0 = s0
		self.stats = {"expd":0, "gend":0, "dups":0, "reopnd":0}

		self.solpath = []
		self.openlist = NodeHeap()
		self.closedlist = {}

		n0 = AbtNode(s0, 0, 0, None)
		self._evalu(n0)
		n0.isopen = True
		
		self.openlist.push(n0)
		self.closedlist[s0] = n0

		while True:
			n = self.openlist.pop()
			n.isopen = False

			if self.dom.checkGoal(n.s) or n.isExact:
				break
			
			self.stats["expd"] += 1

			for c, edgecost in self.dom.expand(n.s):
				if n.parent is not None and c == n.parent.s:
					continue

				self.stats["gend"] += 1
				
				cg = n.g+edgecost
				cdepth = n.depth+1	
				cn = AbtNode(c, cdepth, cg, n)
				self._evalu(cn)
				cn.isopen = True

				if c not in self.closedlist:
					self.openlist.push(cn)
					self.closedlist[c] = cn
					
				else:
					self.stats["dups"] += 1
					dup = self.closedlist[c]
					
					if cn.ug < dup.ug:
						dupstr = str(dup)
						dup.g = cn.g
						dup.depth = cn.depth
						dup.ug = cn.ug
						dup.uf = cn.uf
						dup.parent = n

						if dup.isopen:
							self.openlist.update(dup.openidx)
							
						else:
							self.stats["reopnd"] += 1
							self.openlist.push(dup)
							dup.isopen = True


		self.solpath = [n]

		while self.solpath[0].parent is not None:
			self.solpath.insert(0, self.solpath[0].parent)

		if self.useCaching:
			self._doCaching()

		self.test_sol_g = self.solpath[-1].g
		
		if self.solpath[-1].isExact:
			self.test_sol_remexp = self._remExp(self.solpath[-1].goal_depth)
		else:
			self.test_sol_remexp = self._remExp(self.solpath[-1].depth)

		return self.solpath[-1].uf



class UGSA2:
	def __init__(self, domstack, wf, wt):
		self.doms = [domstack.getDomain(i) for i in range(domstack.getTopLevel()+1)]		
		self.wf = wf
		self.wt = wt
		self.abtSearch = UGSA2_AbtSearch(wf, wt, domstack, 1)
		self.abtSearch_uncached = UGSA2_AbtSearch(wf, wt, domstack, 1, False)
		
	
	def execute(self, s0):
		if s0 is None:
			s0 = self.doms[0].initState()
		
		self.stats = {"expd":0, "gend":0, "dups":0, "reopnd":0}
		self.fcounts = {}
		self.ucounts = {}
		self.fexp = []
		self.uexp = []
		
		self.nxtResort = 16
		self.avgBF = 1
		self.k = 1
		
		self.abtSearch.setParams(1, 1.2)
		self.abtSearch_uncached.setParams(1, 1.2)

		return self._doBaseSearch(s0)



	def compBaseUtil(self, s, g):
		abtstate = self.doms[1].abstractState(s)
		a = self.wf*g + self.abtSearch.doSearch(abtstate)
		#b = self.wf*g + self.abtSearch_uncached.doSearch(abtstate)
		#if a != b:
		#	print s, abtstate, self.wf*g, a, b
		#assert(a == b)
		return a
		
	
	def _doBaseSearch(self, s0):
		dom = self.doms[0]
		
		openlist = NodeHeap()
		closedlist = {}

		n0 = BaseNode(s0, 0, 0, self.compBaseUtil(s0, 0), None)
		n0.test_remexp = self.abtSearch.test_sol_remexp
		n0.test_expn = 0
		n0.isopen = True
		
		openlist.push(n0)
		closedlist[s0] = n0


		while True:
			if self.stats["expd"] >= self.nxtResort:
				self.nxtResort *= 2
				pass
			
			n = openlist.pop()
			
			if dom.checkGoal(n.s):
				goalNode = n
				break
			
			self.stats["expd"] += 1
			
			for c, edgecost in dom.expand(n.s):
				if n.parent is not None and c == n.parent.s:
					continue
			
				self.stats["gend"] += 1
				
				cg = n.g + edgecost
				cdepth = n.depth + 1
				cu = self.compBaseUtil(c, cg)
				
				cn = BaseNode(c, cdepth, cg, cu, n)
				cn.test_remexp = self.abtSearch.test_sol_remexp
				cn.test_expn = self.stats["expd"]
				cn.isopen = True
				
				if c not in closedlist:
					openlist.push(cn)
					closedlist[c] = cn

				else:
					self.stats["dups"] += 1
					dup = closedlist[c]
					
					if cn.g < dup.g:
						dup.g = cn.g
						dup.depth = cn.depth
						dup.u = cn.u
						dup.parent = n
						
						dup.test_remexp = cn.test_remexp
						dup.test_expn = cn.test_expn

						if dup.isopen:
							openlist.update(dup.openidx)							
						else:
							self.stats["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True


		self.solpath = [goalNode]
		while self.solpath[0].parent is not None:
			self.solpath.insert(0, self.solpath[0].parent)

		self.test_expPred = []
		self.test_expd = self.stats["expd"]
		
		for n in self.solpath:
			self.test_expPred.append((n.test_expn, n.test_remexp, (self.test_expd-n.test_expn) - n.test_remexp))

		return goalNode
	

	def _resort(self, openlist):
		for n in openlist.t:
			h, d = self._baseHr(n.s)
			n.u = self._compBaseUtil(n.f, d, n.depth)
		openlist.reinit()



	def _informExpansion(self, n):
		if n.f not in self.fcounts:
			self.fcounts[n.f] = 1
		else:
			self.fcounts[n.f] += 1
		
		if n.u not in self.ucounts:
			self.ucounts[n.u] = 1
		else:
			self.ucounts[n.u] += 1
		
		self.fexp.append((n.f, self.stats[0]["expd"]))
		self.uexp.append((n.u, self.stats[0]["expd"]))

		self.delayInfo[0] += self.stats[0]["expd"] - n.expdAtGen
		self.delayInfo[1] += 1




	def _compAvgBF(self):
		x = [i for i in self.fcounts.iteritems()]
		x.sort()
		flist = [i[1] for i in x]
		if len(flist) <= 3:
			return 1
		
		acc = 0.0
		for i in range(1, len(flist)-2):
			acc += float(flist[i+1]) / flist[i]

		return acc / (len(flist) - 3)


