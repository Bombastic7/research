#!/bin/python

import random

"""
s0 = (ci, cj, ck)
goalpos = (ci, cj, ck)
dropcakes = (lvl c0 is dropped, lvl c1 is, lvl c2 is)
"""



class Domain:
	def __init__(self, lvl, s0, goal, dropcakes):
		assert(len(s0) == len(goal) == len(dropcakes))
		self.lvl = lvl
		self.s0 = tuple(s0)
		self.dropcakes = tuple(dropcakes)
		self.goal = tuple(goal)
		self.randShuffled = range(len(goal))
		
	def expand(self, s):
		children = []
		
		for i in range(1, len(s)):
			c = [None] * len(s)
			
			for j in range(0, i+1):
				c[j] = s[i-j]
			
			for j in range(i+1, len(s)):
				c[j] = s[j]
			
			children.append((tuple(c), 1))

		return children
	

	def initState(self):
		assert(self.lvl == 0)
		return self.s0
	
	
	def randomInitState(self):
		assert(self.lvl == 0)
		random.shuffle(self.randShuffled)
		return tuple(self.randShuffled)

		
	def checkGoal(self, s):
		for i in range(len(s)):
			if s[i] is not None and s[i] != self.goal[i]:
				return False		
		return True

	def drawGoal(self):
		s = [i for i in self.goal]
		for i in range(len(s)):
			if self.dropcakes[s[i]] <= self.lvl:
				s[i] = None
		print s


	def spawnAbtDomain(self):
		tilesRemaining = len(self.dropcakes)
		
		for i in range(len(self.dropcakes)):
			if self.dropcakes[i] <= self.lvl+1:
				tilesRemaining -= 1
		
		if tilesRemaining == 0:
			return None
		return Domain(self.lvl+1, self.s0, self.goal, self.dropcakes)
	

	def abstractState(self, bs):
		s = [i for i in bs]
		for i in range(len(s)):
			if s[i] is not None and self.dropcakes[s[i]] <= self.lvl:
				s[i] = None
		
		return tuple(s)
