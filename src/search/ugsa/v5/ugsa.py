#!/bin/python


from functools import total_ordering




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
			self.t = []
	
		else:
			self.t[0] = self.t.pop()
			self.t[0].openidx = 0
			self._pushdown(0)
		
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
		if l is not None and self.t[l] < self.t[i]:
			sml = l
		if r is not None and self.t[r] < self.t[sml]:
			sml = r
		
		if sml != i:
			self._swap(sml, i)
			return self._pushdown(sml)
		
		return i

	def _swap(self, i, j):
		tmp = self.t[i]
		self.t[i] = self.t[j]
		self.t[j] = tmp
		self.t[i].openidx = i
		self.t[j].openidx = j



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



def astarMain(dom):
	expd = 0
	gend = 0
	dups = 0
	reopnd = 0
	
	openlist = NodeHeap()
	closedlist = {}
	
	s0 = dom.initState()	
	n0 = Node(s0, 0, dom.hval(s0), None)
	n0.isopen = True
	
	openlist.push(n0)
	closedlist[s0] = n0

	while True:
		n = openlist.pop()
		n.isopen = False
		
		if dom.checkGoal(n.s):
			return n, (expd, gend, dups, reopnd)
		
		expd += 1
		
		childnodes = [Node(c, n.g + edgecost, n.g + edgecost + dom.hval(c), n) for (c, edgecost) in dom.expand(n.s)]

		for cn in childnodes:
			if n.parent is not None and cn == n.parent:
				continue
				
			gend += 1
			cn.isopen = True

			if cn.s not in closedlist:
				openlist.push(cn)
				closedlist[cn.s] = cn
			
			else:
				dups += 1
				dup = closedlist[cn.s]
				
				if cn.g < dup.g:
					dup.g = cn.g
					dup.f = cn.f
					dup.parent = cn.parent
					
					if dup.isopen:
						openlist.update(dup.openidx)
					else:
						reopnd += 1
						openlist.push(dup)
						dup.isopen = True














