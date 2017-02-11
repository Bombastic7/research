#!/bin/python

import math
import tiles

from functools import total_ordering
from nodeheap import NodeHeap


@total_ordering
class Node:
	def __init__(self, s, g, f, u, parent):
		self.s = s
		self.g = g
		self.f = f
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



class CompUtil:
	def __init__(self, pBF = None):
		self.fcounts = {}
		self.dirty = False
		self.bf = 1.0
		self.hardBF = pBF
		self.bflist = []

	def informExpansion(self, n):
		if n.u not in self.fcounts:
			self.fcounts[n.u] = 1
		else:
			self.fcounts[n.u] += 1
		self.bflist.append(self.bf)
		self.dirty = True
		

	def compBaseUtil(self, g, h, d, openSz):
		bf = self.getBF()
		return (g+h) + openSz * math.pow(bf, d), (math.pow(bf, d), bf, d)
	
	def compAbtUtil(self, g, depth):
		bf = self.getBF()
		return g + math.pow(bf, depth)
	
	def getBF(self):
		if self.hardBF is not None:
			return self.hardBF
		if self.dirty:
			v = [i for i in self.fcounts.iteritems()]
			#v.sort()
			self.bf = fitExp(v)
			self.dirty = False
		return self.bf



def getRemExpAccuracy(goalNode):
	solpath = [goalNode]
	while solpath[0].parent is not None:
		solpath.insert(0, solpath[0].parent)
	
	for n in solpath:
		realRemExp = goalNode.expd - n.expd
		print solpath.index(n), n.expd, n.info, realRemExp



class UGSA_A:
	def __init__(self, dom, hardBF = None):
		self.doms = [dom, dom.spawnAbtDomain()]
		self.stats = [{"expd":0, "gend":0, "dups":0, "reopnd":0} for i in range(len(self.doms))]
		self.compUtil = CompUtil(hardBF)
		
	def execute(self):
		return self.doBaseSearch(self.doms[0].initState())
	
	def doBaseSearch(self, s0):
		dom = self.doms[0]
		
		openlist = NodeHeap()
		closedlist = {}

		(h0, d0) = self.doAbtSearch(s0)
		u0, info0 = self.compUtil.compBaseUtil(0, h0, d0, 1)
		n0 = Node(s0, 0, h0, u0, None)
		n0.info = info0
		n0.isopen = True
		
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

			self.stats[0]["expd"] += 1
			self.compUtil.informExpansion(n)
			

			for (c, edgecost) in dom.expand(n.s):
				if n.parent is not None and cn == n.parent:
					continue

				self.stats[0]["gend"] += 1

				(h, d) = self.doAbtSearch(c)
				cg = n.g+edgecost
				
				if c not in closedlist:
					cu, info = self.compUtil.compBaseUtil(cg, h, d, openlist.size()+1)
					cn = Node(c, cg, cg+h, cu, n)
					cn.info = info
					cn.isopen = True
					openlist.push(cn)
					closedlist[c] = cn
					
				else:
					self.stats[0]["dups"] += 1
					dup = closedlist[c]
					
					if cg < dup.g:
						dup.g = cg
						dup.f = cg + h
						cu, info = self.compUtil.compBaseUtil(cg, h, d, openlist.size()+1)
						dup.u = cu
						dup.info = info
						dup.parent = n

						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							self.stats[0]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True


	def doAbtSearch(self, bs):
		dom = self.doms[1]
		
		openlist = NodeHeap()
		closedlist = {}
		
		n0 = AbtNode(dom.abstractState(bs), 0, 0, self.compUtil.compAbtUtil(0, 0), None)
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
				return (n.g, n.depth)

			self.stats[1]["expd"] += 1

			for (c, edgecost) in dom.expand(n.s):
				if n.parent is not None and cn == n.parent:
					continue

				self.stats[1]["gend"] += 1

				cg = n.g+edgecost
				cdepth = n.depth+1
				
				if c not in closedlist:
					cn = AbtNode(c, cg, cdepth, self.compUtil.compAbtUtil(cg, cdepth), n)
					cn.isopen = True
					openlist.push(cn)
					closedlist[c] = cn
					
				else:
					self.stats[1]["dups"] += 1
					dup = closedlist[c]
					
					cu = self.compUtil.compAbtUtil(cg, cdepth)
					
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
							




