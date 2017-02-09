#!/bin/python


from functools import total_ordering
from nodeheap import NodeHeap


class BFSearch:
	
	@total_ordering
	class Node:
		def __init__(self, s, depth, parent):
			self.s = s
			self.depth = depth
			self.parent = parent

		def __lt__(self, o):
			return self.depth < o.depth
	
	
	def __init__(self, dom):
		self.dom = dom
	
	def execute(self):
		openlist = NodeHeap()
		closedlist = {}
		
		n0 = BFSearch.Node(self.dom.initState(), 0, None)

		openlist.push(n0)
		closedlist[n0.s] = n0
		
		while True:
			n = openlist.pop()
			
			if self.dom.checkGoal(n.s):
				return n
			
			childnodes = [BFSearch.Node(c, n.depth+1, n) for (c, edgecost) in self.dom.expand(n.s)]

			for cn in childnodes:
				if cn.s not in closedlist:
					openlist.push(cn)
					closedlist[cn.s] = cn



class Astar:
	class Node:
		def __init__(self, s, g, f, parent):
			self.s = s
			self.g = g
			self.f = f
			self.parent = parent

		def __lt__(self, o):
			return self.f < o.f if self.f != o.f else self.g > o.g


	def __init__(self, dom):
		self.dom = dom
		
	def execute(self):
		openlist = NodeHeap()
		closedlist = {}
		stats = {"expd":0, "gend":0, "dups":0, "reopnd":0}
		
		s0 = self.dom.initState()
		n0 = Astar.Node(self.dom.initState(), 0, self.dom.hval(s0), None)
		n0.isopen = True
		
		openlist.push(n0)
		closedlist[n0.s] = n0
		
		while True:
			n = openlist.pop()
			n.isopen = False
			
			if self.dom.checkGoal(n.s):
				return n, stats
			
			stats["expd"] += 1
			
			childnodes = [Astar.Node(c, n.g+edgecost, n.g+edgecost+self.dom.hval(c), n) for (c, edgecost) in self.dom.expand(n.s)]

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
						dup.parent = cn.parent
						
						if dup.isopen:
							openlist.update(dup.openidx)
						else:
							stats[lvl]["reopnd"] += 1
							openlist.push(dup)
							dup.isopen = True
