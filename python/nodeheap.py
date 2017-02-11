#!/bin/python


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
		
		p = (i - 1)//2
		#print self.t[i].f, self.t[p].f
		if self.t[i] < self.t[p]:
			self._swap(i, p)
			return self._pullup(p)
		else:
			return i
	
	def size(self):
		return len(self.t)

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

