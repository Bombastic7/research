#!/bin/python


import heapq
import functools




class NodeHeap:
	def __init__(self):
		self.t = []

	
	def push(self, n):
		n.openidx = len(self.t)
		self.t.append(n)
		self._pullup(n.openidx)

	
	def pop(self):
		if len(self.t) == 0:
			raise ValueError
		
		ret = self.t[0]
		
		if len(self.t) == 1:
			self.t == []
	
		else:
			self.t[0] = self.t.pop()
			self.t[0].openidx = 0
			_pushdown(0)
		
		return ret


	def update(self, i):
		j = self._pullup(i)
		self._pushdown(j)
		

	def _pullup(self, i):
		if i == 0:
			return i
		
		p = (i - 1)/2
		
		if self.t[i] < self.t[p]:
			self._swap(i, p)
			return self._pullup(p)
		else:
			return i
	
	
	def _pushdown(self, i):
		l = i*2 + 1
		r = i*2 + 2

		if l >= len(self.t):
			l = None
		if r >= len(self.t):
			r = None
	
		sml = i
		if l is not None and self[l] < self[i]:
			sml = l
		if r is not None and self[r] < self[sml]:
			sml = r
		
		if sml != i:
			self._swap(sml, i)
			return self._pushdown(sml)
		
		return i

	def _swap(self, i, j):
		tmp = self.t[i]
		self.t[i] = self.t[j]
		self.t[j] = tmp




class Node:
	def __init__(self, s, g, f, parent):
		self.s = s
		self.g = g
		self.f = f
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s
	
	@total_ordering
	def __lt__(self, o):
		if self.f == o.f:
			return self.g > o.g
		
		return self.f < o.f

	def __hash__(self):
		return hash(s)



def astarMain(dom):
	
	openlist = []
	closedlist = set()
	
	s0 = dom.initState()	
	n0 = Node(s0, 0, dom.hval(s0), None)
	n0.isopen = True
	
	heapq.heappush(openlist, n0)
	closedlist.insert(n0)
	
	
	while True:
		n = heapq.heappop(openlist)
		n.isopen = False
		
		if dom.checkGoal(n.s):
			return n
		
		
		childnodes = [Node(c, n.g + edgecost, n.g + edgecost + dom.hval(c), n) for (c, edgecost) in dom.expand(n.s)]
		
		for cn in childnodes:
			if cn not in closedlist:
				heapq.heappush(openlist, cn)
				closedlist.insert(cn)
			
			else:
				dup = closedlist[cn]
				
				if cn.g < dup.g:
					closedlist.remove(dup)
					closedlist.insert(cn)
					
					if dup.isopen:
						
		
