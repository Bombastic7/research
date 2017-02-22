#!/bin/python

from functools import total_ordering
from nodeheap import NodeHeap


@total_ordering
class Node:
	def __init__(self, s, w, x, y, parent):
		self.s = s
		self.w = w
		self.x = x
		self.y = y
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s
	

	def __lt__(self, o):
		if self.y == o.y:
			return self.x > o.x
		
		return self.y < o.y

	def __hash__(self):
		return hash(self.s)


class CacheEntry:
	def __init__(self):
		self.exact = False



class HAstar:
	def __init__(self, domstack, useUnitCost = False, cacheBaseLevel = False):
		self.domstack = domstack
		self.hcaches = [{} for i in range(domstack.getTopLevel()+1)]
		self.useUnitCost = useUnitCost
		self.cacheBaseLevel = cacheBaseLevel

		if not cacheBaseLevel:
			self.hcaches[0] = None

	def execute(self, s0, solPath = True, lvl = 0):
		self.stats = [{"expd":0, "gend":0, "dups":0, "reopnd":0} for i in range(len(self.hcaches))]
		r = self._doSearch(s0, solPath, lvl)
		assert((not self.cacheBaseLevel and lvl == 0) or self.hcaches[lvl][s0].exact)
		if solPath:
			return r


	def getCachedVals(self, s, lvl = 0):
		ent = self.hcaches[lvl][s]
		if self.useUnitCost:
			return ent.sec, ent.prim
		return ent.prim, ent.sec


	def _heval(self, s, lvl):
		if lvl >= len(self.hcaches) - 1:
			return 0
		abtdom = self.domstack.getDomain(lvl+1)
		abtstate = abtdom.abstractState(s)
		return self._doSearch(abtstate, False, lvl+1)

	
	def _doSearch(self, s0, solPath, lvl):
		bestExactNode = None
		dom = self.domstack.getDomain(lvl)
		cache = self.hcaches[lvl]
		
		if not solPath and cache is not None and s0 in cache and cache[s0].exact:
			return cache[s0].prim
		
		openlist = NodeHeap()
		closedlist = {}
		
		if cache is not None:
			if s0 not in cache:
				cache[s0] = CacheEntry()
				cache[s0].prim = self._heval(s0, lvl)
			h0 = cache[s0].prim
		else:
			h0 = self._heval(s0, lvl)

		n0 = Node(s0, 0, 0, h0, None)
		n0.isopen = True

		openlist.push(n0)
		closedlist[s0] = n0

		goalNode = None
		
		while True:
			try:
				n = openlist.pop()
			except ValueError:
				print "error level", lvl, s0
				raise
			
			n.isopen = False

			if dom.checkGoal(n.s) or bestExactNode is n: 
				goalNode = n
				closedlist[n.s] = n
				if dom.checkGoal(n.s):
					assert(n.x == n.y)
				break


			self.stats[lvl]["expd"] += 1
			
			assert(cache is None or cache[n.s].prim is not None)
			
			def heval(s):
				if cache is None:
					return self._heval(s, lvl)
				if s not in cache:
					cache[s] = CacheEntry()
					cache[s].prim = self._heval(s, lvl)
				return cache[s].prim
			
			if self.useUnitCost:
				childnodes = [Node(c, n.w+edgecost, n.x+1, n.x + 1 + heval(c), n) for (c, edgecost) in dom.expand(n.s)]
			else:
				childnodes = [Node(c, n.w+1, n.x+edgecost, n.x+edgecost + heval(c), n) for (c, edgecost) in dom.expand(n.s)]

			for cn in childnodes:
				if n.parent is not None and cn == n.parent:
					continue

				self.stats[lvl]["gend"] += 1
				cn.isopen = True

				if cn.s not in closedlist:
					openlist.push(cn)
					closedlist[cn.s] = cn
					if not solPath and cache[cn.s].exact and (bestExactNode is None or bestExactNode.y > cn.y):
						bestExactNode = cn
						cn.w += cache[cn.s].sec
				
				else:
					self.stats[lvl]["dups"] += 1
					dup = closedlist[cn.s]
					
					if cn.x < dup.x:
						dup.w = cn.w
						dup.x = cn.x
						dup.y = cn.y
						dup.parent = cn.parent
						
						if dup.isopen:
							openlist.update(dup.openidx)
							if not solPath and cache[cn.s].exact and (bestExactNode is None or bestExactNode.y > cn.y):
								bestExactNode = dup
								dup.w += cache[dup.s].sec
						else:
							self.stats[lvl]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True
							if cache is not None:
								assert(not cache[dup.s].exact)


		if cache is not None:
			for n in closedlist.itervalues():
				if n.isopen:
					continue
				
				px = goalNode.y - n.x
				
				if px > cache[n.s].prim:
					cache[n.s].prim = px
			
			m = goalNode
			
			while m is not None:
				if not cache[m.s].exact:
					cache[m.s].exact = True
					cache[m.s].sec = goalNode.w - m.w
				m = m.parent

		assert(cache[goalNode.s].exact)
		assert(cache[s0].exact)
		
		if solPath:
			return goalNode
		return goalNode.y

