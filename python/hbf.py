#!/bin/python

import misc, hastar

from nodeheap import NodeHeap


class InvestigateHBF:
	class Node:
		def __init__(self, s, g, f, depth, parent):
			self.s = s
			self.g = g
			self.f = f
			self.depth = depth
			self.parent = parent

		def __lt__(self, o):
			return self.f < o.f if self.f != o.f else self.g > o.g
	
	
	def __init__(self, dom):
		self.dom = dom
		self.abtalg = hastar.HAstar(dom)


	def execute(self, runs):
		rec = {}
		for i in range(runs):
			s0 = self.dom.randomInitState()
			gn, stats = self.doSearch(s0)
			
			if gn.f not in rec:
				rec[gn.f] = []
			
			rec[gn.f].append(stats["expd"])
			if i%10 == 0:
				print "done", i

		self.rec = rec
		return rec


	def execute_countf(self, runs):
		fcounts = []
		for i in range(runs):
			fc = {}
			s0 = self.dom.randomInitState()
			gn, stats = self.doSearch(s0, fc)
			fcounts.append(fc)
			print i
		return fcounts

	
	def heval(self, s):
		#return self.abtalg.execute(s, 1).f
		return self.dom.hval(s)

	
	def doSearch(self, s0, fcounts = None):
		openlist = NodeHeap()
		closedlist = {}
		stats = {"expd":0, "gend":0, "dups":0, "reopnd":0}

		n0 = InvestigateHBF.Node(s0, 0, self.heval(s0), 0, None)
		n0.isopen = True
		
		openlist.push(n0)
		closedlist[n0.s] = n0
		
		while True:
			n = openlist.pop()
			n.isopen = False
			
			if self.dom.checkGoal(n.s):
				return n, stats
			
			if fcounts is not None:
				if n.f not in fcounts:
					fcounts[n.f] = 0
				fcounts[n.f] += 1
			
			stats["expd"] += 1
			
			childnodes = [InvestigateHBF.Node(c, n.g+edgecost, n.g+edgecost+self.heval(c), n.depth+1, n) for (c, edgecost) in self.dom.expand(n.s)]

			for cn in childnodes:
				if n.parent is not None and cn == n.parent:
					continue
				
				stats["gend"] += 1
				cn.isopen = True
				
				if cn.s not in closedlist:
					openlist.push(cn)
					closedlist[cn.s] = cn
				else:
					stats["dups"] += 1
					dup = closedlist[cn.s]
					if cn.g < dup.g:
						dup.g = cn.g
						dup.f = cn.f
						dup.depth = cn.depth
						dup.parent = cn.parent
						
						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							stats[lvl]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True
