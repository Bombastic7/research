#!/bin/python

import networkx as nx
import random
import math
import matplotlib.pyplot as plt

from functools import total_ordering
from nodeheap import NodeHeap
import hastar



@total_ordering
class Node:
	def __init__(self, s, depth, g, u, parent):
		self.s = s
		self.g = g
		self.u = u
		self.depth = depth
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s

	def __lt__(self, o):
		if self.u != o.u:
			return self.u < o.u
		if self.depth != o.depth:
			return self.depth > o.depth
		return self.g > o.g

	def __hash__(self):
		return hash(self.s)
	


class GraphDomain:
	def __init__(self, grph, goalStates):
		self.grph = grph
		self.goalStates = goalStates


	def states(self):
		return [i for i in self.grph.nodes()]
	
	def drawpos(self, s):
		return s[0], s[1]

	def expand(self, s):
		return [(ngh, self.grph[s][ngh]['weight']) for ngh in self.grph.neighbors(s)]

	def checkGoal(self, s):
		return s in self.goalStates

	def draw(self):
		fig, ax = plt.subplots()
		pos = {}
		edgelabels = {}
		nodecolors = []
		
		for n in self.grph.nodes():
			pos[n] = (n[0], n[1])
			if self.checkGoal(n):
				nodecolors.append('m')
			else:
				nodecolors.append('w')

		for e in self.grph.edges():
			edgelabels[e] = self.grph.edge[e[0]][e[1]]['weight']
			
		nx.draw_networkx(self.grph, pos, with_labels=False, node_color=nodecolors, ax=ax)
		nx.draw_networkx_edge_labels(self.grph, pos, edge_labels=edgelabels, ax=ax)
		


class GraphDomainStack:
	def __init__(self, dom):
		self.dom = dom
		self.grph = self.dom.grph
		self.cheapalg = hastar.HAstar(self, False, True)
		self.shortalg = hastar.HAstar(self, True, True)
		for n in dom.grph.nodes():
			self.cheapalg.execute(n, False, 0)
			self.shortalg.execute(n, False, 0)
			
	def getDomain(self, lvl):
		assert(lvl == 0)
		return self.dom
	
	def getTopLevel(self):
		return 0
	
	def initState(self):
		stateRank = [(self.cheapalg.getCachedVals(n, 0)[0], self.shortalg.getCachedVals(n, 0)[1], n) for n in self.grph]
		stateRank.sort()
		return stateRank[-1][2]


	def draw(self):
		fig, ax = plt.subplots()
		pos = {}
		edgelabels = {}
		nodelabels = {}
		nodecolors = []
		
		for n in self.grph.nodes():
			pos[n] = (n[0], n[1])
			nodelabels[n] = str((self.cheapalg.getCachedVals(n, 0)[0], self.shortalg.getCachedVals(n, 0)[1]))
			
			if self.dom.checkGoal(n):
				nodecolors.append('m')
			else:
				nodecolors.append('w')

		for e in self.grph.edges():
			edgelabels[e] = self.grph.edge[e[0]][e[1]]['weight']
			
		nx.draw_networkx_nodes(self.grph, pos, node_color=nodecolors, ax=ax)
		nx.draw_networkx_edges(self.grph, pos, ax=ax)
		nx.draw_networkx_edge_labels(self.grph, pos, edge_labels=edgelabels, ax=ax)
		
		for i in pos.iterkeys():
			pos[i] = (pos[i][0]-0.25, pos[i][1]-0.25)
		
		nx.draw_networkx_labels(self.grph, pos, labels=nodelabels, ax=ax)
		

def makeGraph1(width = 10, height = 10, goalStates = 4):	
	goals = []
	g = nx.grid_2d_graph(width,height)
	for s,d in g.edges():
		g[s][d]['weight'] = random.randint(0,3)
	
	for i in range(goalStates):
		while True:
			x = (random.randint(0,width-1), random.randint(0,height-1))
			if goals.count(x) == 0:
				break
		goals.append(x)

	return GraphDomainStack(GraphDomain(g, set(goals)))







class UgsaTraverseGraph:
	def __init__(self, domstack, k, bf, wf, wt):
		self.dom = domstack.getDomain(0)
		self.s0 = domstack.initState()
		self.k = k
		self.bf = bf
		self.wf = wf
		self.wt = wt
		
		self.powsumcache = {}
		self.cache_partial = {}
		self.cache_exact = {}
		self.doSearch()


	def powsum(self, n):
		if n not in self.powsumcache:
			acc = 0
			for i in range(n+1):
				acc += math.pow(self.bf, i)
			self.powsumcache[n] = acc
		return self.powsumcache[n]
			
	
	def compUtil(self, g, depth):
		return self.wf*g + self.wt*self.k*self.powsum(depth)

	def _heval(self, n):
		if n.s in self.cache_exact:
			for ent in self.cache_exact[n.s]:
				if 

	def _evalu(self, n):
		if n.s in self.cache_exact:
			for goalState, ent in self.cache_exact[n.s].iteritems():
				if ent[0] <= n.depth and ent[1] >= n.depth:
					n.g += ent[2]
					n.depth += ent[3]
					n.u = self.compUtil(n.g, n.depth)
					n.isExact = True
					n.inferredGoalState = goalState
					return
		
		n.u = self.compUtil(n.g, n.depth)
		n.isExact = False
		
		if n.s in self.cache_partial:
			for ent in self.cache_partial[n.s]:
				for i in range(len(self.cache_partial[n.s])):
					if self.cache_partial[n.s][i][0] <= n.depth and (i+1 == len(self.cache_partial[n.s]) or self.cache_partial[n.s][i+1][0] > n.depth):
						n.u += self.cache_partial[n.s][i][1]


	def doSearch(self, s0 = None):
		self.openlist = NodeHeap()
		self.closedlist = {}
		
		if s0 = None:
			s0 = self.s0
		
		u0_partial = self._heval(s0)
		n0 = Node(s0, 0, 0, self.compUtil(h0,d0), None)
		n0.isopen = True
		
		self.openlist.push(n0)
		self.closedlist[s0] = n0

		while True:
			n = self.openlist.pop()
			n.isopen = False
			
			if self.dom.checkGoal(n.s):
				break

			for c, edgecost in self.dom.expand(n.s):
				if n.parent is not None and c == n.parent.s:
					continue

				h, d = self._heval(c)
				cg = n.g+edgecost
				cdepth = n.depth+1
				cu = self.compUtil(cg + h, cdepth + d)
				

				if c not in self.closedlist:
					cn = Node(c, cdepth, cg, cu, n)
					cn.isopen = True
					self.openlist.push(cn)
					self.closedlist[c] = cn
					
				else:
					dup = self.closedlist[c]
					
					if cu < dup.u:
						dup.g = cg
						dup.depth = cdepth
						dup.u = cu
						dup.parent = n

						if dup.isopen:
							self.openlist.update(dup.openidx)
						else:
							self.openlist.push(dup)
							dup.isopen = True

		self.solpath = [n]

		while self.solpath[0].parent is not None:
			self.solpath.insert(0, self.solpath[0].parent)



		travgrph = nx.Graph()
		for x in self.dom.states():
			travgrph.add_node(x, drawpos=self.dom.drawpos(x), g = "", depth = "", u = "", color='w')
			if self.dom.checkGoal(x) > 0:
				travgrph.node[x]['color'] = 'm'


		for n in self.closedlist.itervalues():
			if n.isopen:
				clr = 'c'
			else:
				clr = 'w'
			if self.solpath.count(n) > 0:
				clr = 'y'


			travgrph.node[n.s]['g'] = str(n.g)
			travgrph.node[n.s]['depth'] = str(n.depth)
			travgrph.node[n.s]['u'] = "{:10.2f}".format(n.u)
			travgrph.node[n.s]['color'] = clr


		for n in self.closedlist.itervalues():
			if n.parent is not None:
				travgrph.add_edge(n.parent.s, n.s, label=str(n.g-n.parent.g))

		self.travgrph = travgrph


	def doCaching(self):
		for n in self.closedlist:
			if n.s not in self.cache_partial:
				self.cache_partial[n.s] = []
			
			foundEntry = False
			for ent in self.cache_partial[n.s]:
				if ent[0] == n.depth:
					foundEntry = True
					if ent[1] < n.u-n.g:
						ent[1] = n.u-n.g
				if foundEntry:
					break
			
			if not foundEntry:
				self.cache_partial[n.s].append([n.depth, n.u-n.g])
				self.cache_partial[n.s].sort(reverse=True)


		for n in self.solpath:
			h = self.solpath[-1].f - n.g
			d = self.solpath[-1].depth - n.depth
			
			if n.s not in self.cache_exact:
				self.cache_exact[n.s] = { self.solpath[-1].s : [n.depth, n.depth, h, d] }
			else:
				if n.depth < self.cache_exact[n.s][self.solpath[-1].s][0]:
					self.cache_exact[n.s][self.solpath[-1].s][0] = n.depth
				elif n.depth > self.cache_exact[n.s][self.solpath[-1].s][1]:
					self.cache_exact[n.s][self.solpath[-1].s][1] = n.depth

				


	def draw(self, lbl):
		fig, ax = plt.subplots()
		pos = {}
		colors = []
		nodelabels = {}
		edgelabels = {}
		
		for n in self.travgrph.nodes():
			pos[n] = self.travgrph.node[n]['drawpos']
			colors.append(self.travgrph.node[n]['color'])
			nodelabels[n] = self.travgrph.node[n][lbl]
		
		for e in self.travgrph.edges():
			edgelabels[e] = self.travgrph.edge[e[0]][e[1]]['label']
			
		nx.draw_networkx_nodes(self.travgrph, pos, node_color=colors, ax=ax)
		nx.draw_networkx_edges(self.travgrph, pos, ax=ax)
		nx.draw_networkx_edge_labels(self.travgrph, pos, edge_labels=edgelabels, ax=ax)
		
		for i in pos.iterkeys():
			pos[i] = (pos[i][0], pos[i][1]-0.25)
		
		nx.draw_networkx_labels(self.travgrph, pos, labels=nodelabels, ax=ax)
		
