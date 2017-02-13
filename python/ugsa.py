#!/bin/python

import math
import tiles

from functools import total_ordering
from nodeheap import NodeHeap



VAR_HBF = "H1"
VAR_BU = "B3"
VAR_AU = None


def trial_A():
	global VAR_HBF
	global VAR_BU
	global VAR_AU
	
	varhbf_opts = ["H1", "H2"]
	varbu_opts = ["B1", "B2"]
	
	weights = [(1,1), (10,1), (100,1), (1000,1)]
	dom = tiles.rand_3_3()
	
	varres = {}
	
	for v in [(v1,v2) for v1 in varhbf_opts for v2 in varbu_opts]:
		VAR_HBF = v[0]
		VAR_BU = v[1]
		varres[v] = makeReport(dom, weights)
	
	return varres
			
	



def makeReport(dom, weights, runs = 10):
	res = []
	for r in range(runs):
		s0 = dom.randomInitState()
		res.append({})
		
		for (wf, wt) in weights:
			alg = UGSA(dom, wf, wt)
			alg.execute(s0)
			
			gn = alg.goalNode
			stats = alg.stats[0]
			
			res[-1][(wf,wt)] = {"cost":gn.g, "length":gn.depth, "expd":stats["expd"]}
			res[-1][(wf,wt)]["utility"] = wf*gn.g + wt*stats["expd"]
	
	return res
			




@total_ordering
class Node:
	def __init__(self, s, g, f, depth, u, parent):
		self.s = s
		self.g = g
		self.f = f
		self.depth = depth
		self.u = u
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
	def __init__(self, s, g, depth, u, parent):
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


class CompUtil:
	def __init__(self, wf, wt, pBF = None):
		self.fcounts = {}
		self.dirty = False
		self.bf = 1.0
		self.hardBF = pBF
		self.exppoints = []
		self.wf = wf
		self.wt = wt
		self.delayInfo = [0.0, 0]
		self.nxtResort = 16
		self.avgDelay = 1
		
		
	def informExpansion(self, n, curexpd):
		if VAR_HBF == "H1":
			x = n.u
		elif VAR_HBF == "H2":
			x = n.f
		
		if x not in self.fcounts:
			self.fcounts[x] = 1
		else:
			self.fcounts[x] += 1
		self.dirty = True
		#self.exppoints.append((n.f, n.u))
		self.delayInfo[0] += curexpd - n.expdAtGen
		self.delayInfo[1] += 1


	def compBaseUtil(self, g, h, d, depth, openlist, curexpd, abtsearch):		
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

	
	def compAbtUtil(self, g, depth, basedepth):
		if VAR_BU == "B1" or VAR_BU == "B2":
			bf = self.getBF()
			return self.wf * g + self.wt * math.pow(bf, depth)
		elif VAR_BU == "B3":
			if self.delayInfo[1] == 0:
				return depth
			return depth * self.avgDelay
		else:
			raise ValueError

	
	def getBF(self):
		if self.hardBF is not None:
			return self.hardBF
		if self.dirty:
			v = [i for i in self.fcounts.iteritems()]
			v.sort()
			#self.bf = fitExp(v[1:-1])
			self.bf = computeAvgBF([i[1] for i in v[1:-1]])
			self.dirty = False
		return self.bf



class UGSA:
	def __init__(self, dom, wf, wt, hardBF = None):
		self.doms = [dom, dom.spawnAbtDomain()]
		self.compUtil = CompUtil(wf, wt, hardBF)
		
	def execute(self, s0 = None):
		if s0 is None:
			s0 = self.doms[0].initState()
		
		self.stats = [{"expd":0, "gend":0, "dups":0, "reopnd":0} for i in range(len(self.doms))]
		self.abtcache = {}
		
		return self.doBaseSearch(s0)
	
	def doBaseSearch(self, s0):
		dom = self.doms[0]
		
		openlist = NodeHeap()
		closedlist = {}

		(h0, d0) = self.doAbtSearch(s0, 0)
		u0 = self.compUtil.compBaseUtil(0, h0, d0, 0, openlist, 0, self.doAbtSearch)
		n0 = Node(s0, 0, h0, 0, u0, None)
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

			self.compUtil.informExpansion(n, self.stats[0]["expd"])
			self.stats[0]["expd"] += 1		

			for (c, edgecost) in dom.expand(n.s):
				if n.parent is not None and cn == n.parent:
					continue

				self.stats[0]["gend"] += 1

				(h, d) = self.doAbtSearch(c, n.depth+1)
				cg = n.g+edgecost
				
				if c not in closedlist:
					cu = self.compUtil.compBaseUtil(cg, h, d, n.depth+1, openlist, self.stats[0]["expd"], self.doAbtSearch)
					cn = Node(c, cg, cg+h, n.depth+1, cu, n)
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
						cu = self.compUtil.compBaseUtil(cg, h, d, n.depth+1, openlist, self.stats[0]["expd"], self.doAbtSearch)
						dup.u = cu
						dup.depth = n.depth+1
						dup.parent = n
						dup.expdAtGen = self.stats[0]["expd"]

						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							self.stats[0]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True


	def doAbtSearch(self, bs, basedepth):
		dom = self.doms[1]
		s0 = dom.abstractState(bs)
		
		if s0 in self.abtcache:
			if self.stats[0]["expd"] - self.abtcache[s0][1] < self.stats[0]["expd"] * 0.25:
				return self.abtcache[s0][0]

		openlist = NodeHeap()
		closedlist = {}
		
		
		n0 = AbtNode(s0, 0, 0, self.compUtil.compAbtUtil(0, 0, basedepth), None)
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
				self.abtcache[s0] = ((n.g, n.depth), self.stats[0]["expd"])
				return (n.g, n.depth)

			self.stats[1]["expd"] += 1

			for (c, edgecost) in dom.expand(n.s):
				if n.parent is not None and cn == n.parent:
					continue

				self.stats[1]["gend"] += 1

				cg = n.g+edgecost
				cdepth = n.depth+1
				
				if c not in closedlist:
					cn = AbtNode(c, cg, cdepth, self.compUtil.compAbtUtil(cg, cdepth, basedepth), n)
					cn.isopen = True
					openlist.push(cn)
					closedlist[c] = cn
					
				else:
					self.stats[1]["dups"] += 1
					dup = closedlist[c]
					
					cu = self.compUtil.compAbtUtil(cg, cdepth, basedepth)
					
					if cu < dup.u:
						dup.g = cg
						dup.depth = cdepth
						dup.u = cu
						dup.parent = n

						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							self.stats[1]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True
							




