#!/bin/python

import misc

from functools import total_ordering
from nodeheap import NodeHeap


@total_ordering
class Node:
	def __init__(self, s, g, f, parent):
		self.s = s
		self.g = g
		self.f = f
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s
	

	def __lt__(self, o):
		if self.f == o.f:
			return self.g > o.g
		
		return self.f < o.f

	def __hash__(self):
		return hash(self.s)






class HAstar:
	def __init__(self, dom):
		self.doms = [(dom, None)]
		while True:
			abtdom = self.doms[-1][0].spawnAbtDomain()
			if abtdom is None:
				break
			self.doms.append((abtdom, {}))


	def execute(self, s0 = None, lvl = 0):
		if s0 is None:
			s0 = self.doms[0][0].initState()
		
		self.stats = [{"expd":0, "gend":0, "dups":0, "reopnd":0} for i in range(len(self.doms))]
		return self.doSearch(s0, lvl)
	
	
	def doSearch(self, bs, lvl):
		if lvl >= len(self.doms):
			return Node(bs,0,0,None)

		bestExactNode = None
		
		dom = self.doms[lvl][0]
		cache = self.doms[lvl][1]
		
		s0 = dom.abstractState(bs) if lvl > 0 else bs
		
		if cache is not None and s0 in cache and cache[s0][1]:
			return Node(bs,0,cache[s0][0],None)
		
		openlist = NodeHeap()
		closedlist = {}
		
		if cache is not None:
			if s0 not in cache:
				cache[s0] = [self.doSearch(s0, lvl+1).f, False]
			h0 = cache[s0][0]
		else:
			h0 = self.doSearch(s0, lvl+1).f
			assert(lvl == 0)

		n0 = Node(s0, 0, h0, None)
		n0.isopen = True

		openlist.push(n0)
		closedlist[s0] = n0

		goalNode = None
		
		while True:
			try:
				n = openlist.pop()
			except ValueError:
				print "error level", lvl
				raise
			
			n.isopen = False

			if dom.checkGoal(n.s) or (lvl != 0 and bestExactNode is n): 
				goalNode = n
				if dom.checkGoal(n.s):
					assert(n.g == n.f)
				break


			self.stats[lvl]["expd"] += 1
			
			def heval(s):
				if lvl == 0:
					return self.doSearch(s, 1).f
				if s not in cache:
					cache[s] = [self.doSearch(s, lvl+1).f, False]
				return cache[s][0]
			
			childnodes = [Node(c, n.g + edgecost, n.g + edgecost + heval(c), n) for (c, edgecost) in dom.expand(n.s)]

			for cn in childnodes:

				if n.parent is not None and cn == n.parent:
					continue
					
				self.stats[lvl]["gend"] += 1
				cn.isopen = True

				if cn.s not in closedlist:
					openlist.push(cn)
					closedlist[cn.s] = cn
					if lvl != 0 and cache[cn.s][1] and (bestExactNode is None or bestExactNode.f > cn.f):
						bestExactNode = cn
				
				else:
					self.stats[lvl]["dups"] += 1
					dup = closedlist[cn.s]
					
					if cn.g < dup.g:
						dup.g = cn.g
						dup.f = cn.f
						dup.parent = cn.parent
						
						if dup.isopen:
							openlist.update(dup.openidx)
							if lvl != 0 and cache[cn.s][1] and (bestExactNode is None or bestExactNode.f > cn.f):
								bestExactNode = dup
						else:
							self.stats[lvl]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True
							if cache is not None:
								assert(not cache[s0][1])


		if lvl > 0:
			for n in closedlist.itervalues():
				if n.isopen:
					continue
				
				pg = goalNode.f - n.g
				
				if pg > cache[n.s][0]:
					cache[n.s][0] = pg
			
			m = goalNode
			
			while m is not None:
				cache[m.s][1] = True
				m = m.parent
		
		if lvl == 0:
			self.goalNode = goalNode

		return goalNode



