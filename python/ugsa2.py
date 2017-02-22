#!/bin/python

import math
import hastar
import gridnav

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





class UGSA2:
	def __init__(self, domstack):
		self.doms = [domstack.getDomain(i) for i in range(domstack.getTopLevel()+1)]
		self.abtalg_short = hastar.HAstar(domstack, True)
		self.abtalg_cheap = hastar.HAstar(domstack)
		self.powsumcache = {}
	
		
	def _powsum(self, n):
		if n not in self.powsumcache:
			acc = 0
			for i in range(n+1):
				acc += math.pow(self.avgBF, i)
			self.powsumcache[n] = acc
		return self.powsumcache[n]
			
	
	
	def execute(self, s0, wf = 1, wt = 1, useBF = False):
		if s0 is None:
			s0 = self.doms[0].initState()
		
		self.stats = [{"expd":0, "gend":0, "dups":0, "reopnd":0} for i in range(len(self.doms))]
		self.fcounts = {}
		self.ucounts = {}
		self.fexp = []
		self.uexp = []
		self.delayInfo = [0.0, 0]
		self.nxtResort = 16
		self.avgDelay = 1
		self.avgBF = 1
		
		self.wf = wf
		self.wt = wt
		self.useBF = useBF

		return self._doBaseSearch(s0)


	def getStats(self):
		stats = {"stats":self.stats}
		#stats["fcounts"] = self.fcounts
		#stats["ucounts"] = self.ucounts
		#stats["fexp"] = self.fexp
		#stats["uexp"] = self.uexp
		stats["delayInfo"] = self.delayInfo
		stats["nxtResort"] = self.nxtResort
		stats["avgDelay"] = self.avgDelay
		stats["avgBF"] = self.avgBF
		stats["goalnode"] = (self.goalNode.depth, self.goalNode.g, self.goalNode.f, self.goalNode.u)
		return stats



	def _baseHr(self, s):



	def _abtHr(self, abt1state):
		if len(self.doms) < 3:
			return 0, 0

		abt2state = self.doms[2].abstractState(abt1state)
		self.abtalg_cheap.execute(abt2state, 2)
		self.abtalg_short.execute(abt2state, 2)

		return self.abtalg_cheap.getCachedVals(abt2state, 2)[0], self.abtalg_short.getCachedVals(abt2state, 2)[1]

	
	
	def _doBaseSearch(self, s0):
		dom = self.doms[0]
		
		openlist = NodeHeap()
		closedlist = {}

		h0, d0 = self._baseHr(s0)
		n0 = Node(s0, 0, 0, h0, None)
		n0.u = self._compBaseUtil(h0, d0, 0)
		n0.isopen = True
		n0.expdAtGen = 0
		
		openlist.push(n0)
		closedlist[s0] = n0

		while True:
			if self.stats[0]["expd"] >= self.nxtResort:
				self.avgDelay = self.delayInfo[0] / self.delayInfo[1]
				self.delayInfo = [0.0,0]
				self.nxtResort *= 2
				
				self.avgBF = self._compAvgBF()
				self._resort(openlist)
				
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

				h, d = self._baseHr(c)
				cg = n.g+edgecost
				
				if c not in closedlist:
					cn = Node(c, n.depth+1, cg, cg+h, n)
					cn.u = self._compBaseUtil(cg+h, d, cn.depth)
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
						dup.u = self._compBaseUtil(dup.f, d, dup.depth)
						dup.expdAtGen = self.stats[0]["expd"]

						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							self.stats[0]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True


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




	def _compBaseUtil(self, f, d, depth):
		if not self.useBF:
			return f * self.wf + d * self.avgDelay * self.wt
		else:
			if self.useBFwithDepth:
				remexp = math.pow(self.avgBF, depth+d)
			else:
				remexp = math.pow(self.avgBF, d)
			return f * self.wf + remexp * self.wt 



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



def runtests():
	gridnavstack = gridnav.inst_1000()
	testalg = UGSA2(gridnavstack)
	s0 = 845+346*1000

	results = {(1,1):{}, (10,1):{}, (100,1):{}, (1000,1):{}}


	for wf, wt in results.iterkeys():
			
		key = "delay"
		testalg.execute(s0, wf, wt)
		results[(wf,wt)][key] = testalg.getStats()
		print "done", (wf,wt), key
		
		key = "bf"
		testalg.execute(s0, wf, wt, True)
		results[(wf,wt)][key] = testalg.getStats()
		print "done", (wf,wt), key
		
		key = "bf_depth"
		testalg.execute(s0, wf, wt, True, True)
		results[(wf,wt)][key] = testalg.getStats()
		print "done", (wf,wt), key

	return results


def extractResults(results):
	lst = {}
	for w, v1 in results.iteritems():
		lst[w] = {}
		for k, v2 in v1.iteritems():
			lst[w][k] = v2['avgBF']
	return lst
