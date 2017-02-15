#!/bin/python

import math
import tiles
import hastar

from functools import total_ordering
from nodeheap import NodeHeap






@total_ordering
class Node:
	def __init__(self, s, depth, g, f, parent):
		self.s = s
		self.g = g
		self.f = f
		self.depth = depth
		self.u = None
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s

	def __lt__(self, o):
		if self.u != o.u:
			return self.u < o.u
		if self.f != o.f:
			return self.f < o.f
		return self.g > o.g
		

	def __hash__(self):
		return hash(self.s)


@total_ordering
class AbtNode:
	def __init__(self, s, depth, g, parent):
		self.s = s
		self.g = g
		self.depth = depth
		#self.u = None
		#self.uf = None
		self.parent = parent
	
	def __eq__(self, o):
		return self.s == o.s
	
	def __lt__(self, o):
		if self.uf != o.uf:
			return self.uf < o.uf
		if self.u != o.u:
			return self.u > o.u
		if self.depth != o.depth:
			return self.depth < o.depth
		return self.g < o.g
	
	def __hash__(self):
		return hash(self.s)



def fitExp(v):
	if len(v) == 0:
		return 1.0
	#v = list of (x,y) pairs
	z = [math.log(y) / x for (x,y) in v]
	avg = math.fsum(z) / len(z)
	return math.exp(avg)


def computeAvgBF(lst):
	if len(lst) <= 1:
		return 1.0
	acc = 0.0
	for i in range(len(lst)-1):
		acc += float(lst[i+1]) / lst[i]
	return acc / (len(lst)-1)





class UGSA:
	def __init__(self, domstack, wf, wt, hardBF = None):
		self.doms = [domstack.getDomain(i) for i in range(domstack.getTopLevel()+1)]
		self.wf = wf
		self.wt = wt
		self.hardBF = hardBF
		self.abt2alg_h = hastar.HAstar(domstack)
		self.abt2alg_d = hastar.HAstar(domstack, True)
		
	def execute(self, s0 = None):
		if s0 is None:
			s0 = self.doms[0].initState()
		
		self.stats = [{"expd":0, "gend":0, "dups":0, "reopnd":0} for i in range(len(self.doms))]
		self.abtcache = {}
		self.fcounts = {}
		self.ucounts = {}
		self.fexp = []
		self.uexp = []
		self.delayInfo = [0.0, 0]
		self.nxtResort = 16
		self.avgDelay = 1
		self.bf = 1.0
		self.dirty = False
		
		self.var_hbf_countattr = lambda n: n.f
		
		
		return self._doBaseSearch(s0)

	
	
	def _doBaseSearch(self, s0):
		dom = self.doms[0]
		
		openlist = NodeHeap()
		closedlist = {}

		(h0, d0) = self._doAbtSearch(s0, 0)
		n0 = Node(s0, 0, 0, h0, None)
		n0.u = self._compBaseUtil(n0)
		
		n0.isopen = True
		n0.expdAtGen = 0
		
		openlist.push(n0)
		closedlist[s0] = n0
		
		while True:
			try:
				n = openlist.pop()
			except ValueError:
				print "error level 0"
				raise
			
			n.isopen = False
			n.expd = self.stats[0]["expd"]
			
			if dom.checkGoal(n.s):
				self.goalNode = n
				return n

			self._informExpansion(n)
			self.stats[0]["expd"] += 1		

			for (c, edgecost) in dom.expand(n.s):
				if n.parent is not None and c == n.parent.s:
					continue

				self.stats[0]["gend"] += 1

				(h, d) = self._doAbtSearch(c, n.depth+1)
				cg = n.g+edgecost
				
				if c not in closedlist:
					cn = Node(c, n.depth+1, cg, cg+h, n)
					cn.u = self._compBaseUtil(cn)
					cn.isopen = True
					cn.expdAtGen = self.stats[0]["expd"]
					openlist.push(cn)
					closedlist[c] = cn
					
				else:
					self.stats[0]["dups"] += 1
					dup = closedlist[c]
					
					if cg < dup.g:
						dup.g = cg
						dup.f = cg + h
						dup.depth = n.depth+1
						dup.parent = n
						dup.u = self._compBaseUtil(dup)
						dup.expdAtGen = self.stats[0]["expd"]

						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							self.stats[0]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True


	def _doAbtSearch(self, bs, basedepth):
		dom = self.doms[1]
		s0 = dom.abstractState(bs)
		"""
		if s0 in self.abtcache:
			if self.stats[0]["expd"] - self.abtcache[s0][1] < self.stats[0]["expd"] * 0.25:
				return self.abtcache[s0][0]
		"""
		openlist = NodeHeap()
		closedlist = {}

		n0 = AbtNode(s0, 0, 0, None)
		n0.u = self._compAbtUtil(basedepth, n0)
		n0.isopen = True
		openlist.push(n0)
		closedlist[n0.s] = n0
		
		while True:
			try:
				n = openlist.pop()
			except ValueError:
				print "error level 1"
				raise
			
			n.isopen = False

			if dom.checkGoal(n.s):
				#self.abtcache[s0] = ((n.g, n.depth), self.stats[0]["expd"])
				return (n.g, n.depth)

			self.stats[1]["expd"] += 1

			for (c, edgecost) in dom.expand(n.s):
				if n.parent is not None and c == n.parent.s:
					continue

				self.stats[1]["gend"] += 1

				cg = n.g+edgecost
				cdepth = n.depth+1
				
				cn = AbtNode(c, cdepth, cg, n)
				cn.u = self._compAbtUtil(basedepth, cn)
				cn.uf = self._compAbtGoalUtil(cn)
				assert(cn.u <= cn.uf)
				cn.isopen = True
				
				if c not in closedlist:
					openlist.push(cn)
					closedlist[c] = cn
				else:
					self.stats[1]["dups"] += 1
					dup = closedlist[c]

					if cn.u < dup.u:
						dup.g = cn.g
						dup.depth = cn.depth
						dup.u = cn.u
						dup.uf = cn.uf
						dup.parent = cn.parent
						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							self.stats[1]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True
							


	def _compAbtGoalUtil(self, n):
		abtstate = self.doms[2].abstractState(n.s)
		
		self.abt2alg_h.execute(abtstate, 2)
		cheap_h, cheap_d = self.abt2alg_h.getCachedVals(abtstate, 2)
		
		self.abt2alg_d.execute(abtstate, 2)
		short_d, short_h = self.abt2alg_d.getCachedVals(abtstate, 2)
		
		cheapGoalUtil = self._compAbtUtil(0, nodeVals = (n.depth + cheap_d, n.g + cheap_h))
		shortGoalUtil = self._compAbtUtil(0, nodeVals = (n.depth + short_d, n.g + short_h))
		
		return max(cheapGoalUtil, shortGoalUtil)


	def _informExpansion(self, n):
		#x = self.var_hbf_countattr(n)
		
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
		
		self.dirty = True
		#self.exppoints.append((n.f, n.u))
		self.delayInfo[0] += self.stats[0]["expd"] - n.expdAtGen
		self.delayInfo[1] += 1


	def _compBaseUtil(self, n):
		"""
		if VAR_BU == "B1":
			remexp = math.pow(self.getBF(), d) * openlist.size()
		elif VAR_BU == "B2":
			remexp = math.pow(self.getBF(), depth+d)
		elif VAR_BU == "B3":
			if curexpd >= self.nxtResort:
				self.nxtResort *= 2
				self.avgDelay = self.delayInfo[0] / self.delayInfo[1]
				
				for n in openlist.t:
					(hn, dn) = abtsearch(n.s, n.depth)
					n.f = n.g + hn
					n.u = self.wf * n.f + self.wt * self.avgDelay * dn

			remexp = d * self.avgDelay

		else:
			print VAR_BU
			raise ValueError

		return self.wf * (g+h) + self.wt * remexp
		"""
		return n.f


	
	def _compAbtUtil(self, basedepth, n = None, nodeVals = None):
		"""
		if VAR_BU == "B1" or VAR_BU == "B2":
			bf = self.getBF()
			return self.wf * g + self.wt * math.pow(bf, depth)
		elif VAR_BU == "B3":
			if self.delayInfo[1] == 0:
				return depth
			return depth * self.avgDelay
		else:
			raise ValueError
		"""
		if n is not None:
			return n.g
		if nodeVals is not None:
			return nodeVals[1]

	
	def _getBF(self):
		"""
		if self.hardBF is not None:
			return self.hardBF
		if self.dirty:
			v = [i for i in self.fcounts.iteritems()]
			v.sort()
			#self.bf = fitExp(v[1:-1])
			self.bf = computeAvgBF([i[1] for i in v[1:-1]])
			self.dirty = False
		return self.bf
		"""
