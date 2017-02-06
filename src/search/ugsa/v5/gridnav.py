#!/bin/python



CELL_OPEN = 0
CELL_BLOCKED = 1



class State:
	def __init__(self, idx):
		self.idx = idx

	def __eq__(self, o):
		return self.idx == o.idx

	def __hash__(self):
		return self.idx


class Domain:
	def __init__(self, fmapname, s0, goal, height, width):
		self.fmapname = fmapname
		self.s0 = s0
		self.goal = goal
		self.height = height
		self.width = width
		self.size = height * width
		
		self.cells = [None] * (self.size)
		
		with open(fmapname) as f:
			s = f.read().split()
			
			for i in range(self.size):
				self.cells[i] = int(s[i])
				assert(self.cells[i] == CELL_OPEN or self.cells[i] == CELL_BLOCKED)

		assert(self.cells[s0] == CELL_OPEN)
		assert(self.cells[goal] == CELL_OPEN)


	def expand(self, s):
		adjcells = []
		if s.idx >= self.width:
			adjcells.append((State(s.idx - self.width), 1))
		
		if s.idx < (self.height - 1) * self.width:
			adjcells.append((State(s.idx + self.width), 1))

		if s.idx % self.width != 0:
			adjcells.append((State(s.idx - 1), 1))

		if (s.idx + 1) % self.width != 0:
			adjcells.append((State(s.idx + 1), 1))

		return [c for c in adjcells if self.cells[c] == CELL_OPEN]


	def initState(self):
		return State(self.s0)

	
	def checkGoal(self, s):
		return s.idx == self.goal
	
	
	def hval(self, s):
		return 0
