#!/bin/python

import random

class Domain:
	def __init__(self, lvl, height, width, s0, goal, droptiles):
		self.lvl = lvl
		self.height = height
		self.width = width
		self.s0 = s0
		self.goal = goal
		self.size = height*width
		self.droptiles = droptiles
		assert(len(s0) == len(goal) == len(droptiles) == self.size)


	def initState(self):
		if self.lvl == 0:
			return self.s0
		if self.lvl == 1:
			return self.abstractState(self.s0)
	

	def randomInitState(self):
		assert(self.lvl == 0)
		return tuple(genRandInitState(self.goal, self.width))


	def expand(self, s):
		children = []
		blankpos = s.index(0)
		
		if blankpos >= self.width:
			c = list(s)
			c[blankpos] = c[blankpos-self.width]
			c[blankpos-self.width] = 0
			children.append((tuple(c),1))

		if blankpos < (self.height-1) * self.width:
			c = list(s)
			c[blankpos] = c[blankpos+self.width]
			c[blankpos+self.width] = 0
			children.append((tuple(c),1))
			
		if blankpos % self.width != 0:
			c = list(s)
			c[blankpos] = c[blankpos-1]
			c[blankpos-1] = 0
			children.append((tuple(c),1))
		
		if (blankpos + 1) % self.width != 0:
			c = list(s)
			c[blankpos] = c[blankpos+1]
			c[blankpos+1] = 0
			children.append((tuple(c),1))
		
		return children
	
	
	def checkGoal(self, s):
		for i in range(self.size):
			if s[i] is not None and s[i] != self.goal[i]:
				return False			
		return True


	def spawnAbtDomain(self):
		remainingTiles = len(self.s0)
		for i in self.droptiles:
			if i <= self.lvl:
				remainingTiles -= 1
		
		if remainingTiles == 1:
			return None
		return Domain(self.lvl+1, self.height, self.width, self.s0, self.goal, self.droptiles)


	def abstractState(self, bs):
		s = [i for i in bs]
		for i in range(len(bs)):
			if s[i] is not None and self.droptiles[s[i]] <= self.lvl:
				s[i] = None
		return tuple(s)

	
	def drawState(self, s):
		for ln in [list(s[i:i+self.width]) for i in range(0, self.size, self.width)]:
			for i in range(self.width):
				if ln[i] is None:
					ln[i] = " "
				else:
					ln[i] = str(ln[i])
			print ln

	def hval(self, s):
		assert(self.lvl == 0)
		d = 0
		for i in range(1, self.size):
			x = s.index(i) % self.width
			y = s.index(i) / self.width
			gx = self.goal.index(i) % self.width
			gy = self.goal.index(i) / self.width
			d += abs(x-gx) + abs(y-gy)
		return d



def getParity(s, width):
	n = 0
	for i in range(len(s)):
		for j in range(i+1, len(s)):
			if(s[i] > s[j] and s[j] != 0):
				n += 1
	
	if width%2 == 1:
		return n%2 == 0
	
	return (s.index(0)/width) % 2 == n % 2


def genRandInitState(goal, width):
	gpar = getParity(goal, width)
	s0 = range(len(goal))
	random.shuffle(s0)
	while getParity(s0, width) != gpar:
		random.shuffle(s0)
	return s0


def rand_3_3():
	s0 = genRandInitState(range(9), 3)
	return Domain(0, 3, 3, tuple(s0), tuple(range(9)), (10, 5, 4, 3, 2, 1, 1, 1, 1))


def rand_3_4():
	s0 = genRandInitState(range(12), 4)
	return Domain(0, 3, 4, tuple(s0), tuple(range(12)), (10, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1))


def rand_4_4():
	s0 = genRandInitState(range(16), 4)
	return Domain(0, 4, 4, tuple(s0), tuple(range(16)), (10, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1))


