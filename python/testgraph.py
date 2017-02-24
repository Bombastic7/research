#!/bin/python

import networkx as nx
import random
import math
import matplotlib.pyplot as plt

from functools import total_ordering
from nodeheap import NodeHeap
import hastar
import sys


@total_ordering
class Node:
	def __init__(self, s, g, depth, parent):
		self.s = s
		self.g = g
		self.depth = depth
		self.parent = parent

	def __eq__(self, o):
		return self.s == o.s

	def __lt__(self, o):
		if self.uf != o.uf:
			return self.uf < o.uf
		if self.ug != o.ug:
			return self.ug > o.ug
		if self.g != o.g:
			return self.g > o.g
		return self.depth > o.depth

	def __hash__(self):
		return hash(self.s)
	
	def __str__(self):
		s = "[" + str(self.s) + " " + str(self.g) + " " + str(self.depth)
		if hasattr(self, "ug"):
			s += " " + str(self.ug) + " " + str(self.uf)
		s += "]"
		return s


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
	def __init__(self, domstack, k, bf, wf, wt, useCache = True, drawGraph = True):
		self.dom = domstack.getDomain(0)
		self.s0 = domstack.initState()
		self.k = k
		self.bf = bf
		self.wf = wf
		self.wt = wt
		self.useCache = useCache
		self.drawGraph = drawGraph
		
		self.powsumcache = {}
		self.cache_partial = {}
		self.cache_exact = {}
		self.specGoal = None
		
		if useCache:
			self.uncachedAlg = UgsaTraverseGraph(domstack, k, 1.2, 1, 1, False, False)

		self.fig = None
		self.ax = None
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


	def _evalu(self, n):
		n.ug = self.compUtil(n.g, n.depth)
		n.uf = n.ug
		n.isExact = False
		
		if not self.useCache:
			return

		if n.s in self.cache_exact:
			for goalState, ent in self.cache_exact[n.s].iteritems():
				if ent[0] <= n.depth and ent[1] >= n.depth:
					if self.drawGraph:
						print "::", n, ent
					
					n.goal_g = n.g + ent[2]
					n.goal_depth = n.depth + ent[3]
					n.uf = self.compUtil(n.goal_g, n.goal_depth)
					n.isExact = True
					n.inferredGoalState = goalState
					if self.drawGraph:
						print "cache exact", n, n.inferredGoalState
					return

		if n.s in self.cache_partial:
			x = [e for e in self.cache_partial[n.s].iteritems() if e[0] <= n.depth]
			if len(x) > 0:
				x.sort()
				n.uf += x[-1][1]


	def doSearch2(self, s0, sg):
		self.specGoal = sg
		r = self.doSearch(s0)
		self.specGoal = None
		return r

	def doSearch(self, s0 = None, initg = 0, initdepth = 0, printGoalNode = False):
		self.solpath = []
		self.openlist = NodeHeap()
		self.closedlist = {}
		
		if s0 is None:
			s0 = self.s0
		else:
			self.s0 = s0

		n0 = Node(s0, initg, initdepth, None)
		self._evalu(n0)
		n0.isopen = True
		
		self.openlist.push(n0)
		self.closedlist[s0] = n0

		while True:
			n = self.openlist.pop()
			n.isopen = False
			
			""""""
			if self.drawGraph:
				self.draw(['depth'], n)
				print "\nSelect", n
				raw_input()
			""""""

			if self.dom.checkGoal(n.s) or n.isExact or (self.specGoal is not None and self.specGoal == n.s):
				break

			for c, edgecost in self.dom.expand(n.s):
				if n.parent is not None and c == n.parent.s:
					""""""
					if self.drawGraph:
						print "pruned", c, edgecost
					""""""
					continue

				cg = n.g+edgecost
				cdepth = n.depth+1	
				cn = Node(c, cg, cdepth, n)
				self._evalu(cn)
				cn.isopen = True

				""""""
				if self.drawGraph:
					sys.stdout.write(str(cn) + ": ")
				""""""
				
				if c not in self.closedlist:
					self.openlist.push(cn)
					self.closedlist[c] = cn
					""""""
					if self.drawGraph:
						print "add"
					""""""
					
				else:
					dup = self.closedlist[c]
					
					if cn.ug < dup.ug:
						dupstr = str(dup)
						dup.g = cn.g
						dup.depth = cn.depth
						dup.ug = cn.ug
						dup.uf = cn.uf
						self._evalu(dup)
						dup.parent = n

						if dup.isopen:
							self.openlist.update(dup.openidx)
							""""""
							if self.drawGraph:
								print "replace dup", dupstr
							""""""
							
						else:
							self.openlist.push(dup)
							dup.isopen = True
							""""""
							if self.drawGraph:
								print "replace dup (reopen)", dupstr
							""""""
							
					else:
						""""""
						if self.drawGraph:
							print "prune dup"
						""""""

		self.solpath = [n]

		while self.solpath[0].parent is not None:
			self.solpath.insert(0, self.solpath[0].parent)

		self.doCaching()
		
		if self.specGoal is not None:
			assert(self.specGoal == self.solpath[-1].s)

		if printGoalNode:
			print self.solpath[-1]
		return self.solpath[-1].uf


	def doCaching(self):
		
		if self.solpath[-1].isExact:
			goalState = self.solpath[-1].inferredGoalState
			goal_g = self.solpath[-1].goal_g
			goal_depth = self.solpath[-1].goal_depth
		else:
			goalState = self.solpath[-1].s
			goal_g = self.solpath[-1].g
			goal_depth = self.solpath[-1].depth

		goal_ug = self.compUtil(goal_g, goal_depth)
		
		for n in self.closedlist.itervalues():
			if n.isopen:
				continue
			if n.s not in self.cache_partial:
				self.cache_partial[n.s] = {}
			
			pu = goal_ug - n.ug
			
			
			if n.depth not in self.cache_partial[n.s]:
				self.cache_partial[n.s][n.depth] = 0
			
			if self.cache_partial[n.s][n.depth] < pu:
				self.cache_partial[n.s][n.depth] = pu


		
		for n in self.solpath:
			if n.isExact:
				if n is self.solpath[-1]:
					continue
				else:
					raise ValueError

			h = goal_g - n.g
			d = goal_depth - n.depth

			
			if n.s not in self.cache_exact:
				self.cache_exact[n.s] = {}

			if goalState not in self.cache_exact[n.s]:
				self.cache_exact[n.s][goalState] = [n.depth, n.depth, h, d]
			else:
				try:
					if n.depth < self.cache_exact[n.s][goalState][0]:
						self.cache_exact[n.s][goalState][0] = n.depth
					elif n.depth > self.cache_exact[n.s][goalState][1]:
						self.cache_exact[n.s][goalState][1] = n.depth
				except:
					print self.s0, n.s, goalState
					m = self.solpath[-1]
					while m is not None:
						print m
						m = m.parent
					raise
					


	def draw(self, lbls, poppedNode = None):
		if self.fig is None:
			self.fig, self.ax = plt.subplots()
		self.ax.clear()

		pos = {}
		colors = []
		nodelabels = {}
		edgelabels = {}
		
		travgrph = nx.Graph()
		for x in self.dom.states():
			travgrph.add_node(x, drawpos=self.dom.drawpos(x), g = "", depth = "", ug = "", uf="", color='w')
			if self.dom.checkGoal(x) > 0:
				travgrph.node[x]['color'] = 'm'


		for n in self.closedlist.itervalues():
			if n.isopen:
				clr = 'c'
			else:
				clr = 'w'
			if self.solpath.count(n) > 0:
				clr = 'y'
			if n is poppedNode:
				clr = 'r'

			travgrph.node[n.s]['g'] = str(n.g)
			travgrph.node[n.s]['depth'] = str(n.depth)
			travgrph.node[n.s]['ug'] = "{:.2f}".format(n.ug)
			travgrph.node[n.s]['uf'] = "{:.2f}".format(n.uf)
			travgrph.node[n.s]['color'] = clr


		for n in self.closedlist.itervalues():
			if n.parent is not None:
				travgrph.add_edge(n.parent.s, n.s, label=str(n.g-n.parent.g))

		self.travgrph = travgrph
		
		
		for n in self.travgrph.nodes():
			pos[n] = self.travgrph.node[n]['drawpos']
			colors.append(self.travgrph.node[n]['color'])
			nodelabels[n] = str([self.travgrph.node[n][lbl] for lbl in lbls])
		
		for e in self.travgrph.edges():
			edgelabels[e] = self.travgrph.edge[e[0]][e[1]]['label']
			
		nx.draw_networkx_nodes(self.travgrph, pos, node_color=colors, ax=self.ax)
		nx.draw_networkx_edges(self.travgrph, pos, ax=self.ax)
		nx.draw_networkx_edge_labels(self.travgrph, pos, edge_labels=edgelabels, ax=self.ax)
		
		for i in pos.iterkeys():
			pos[i] = (pos[i][0], pos[i][1]-0.25)
		
		nx.draw_networkx_labels(self.travgrph, pos, labels=nodelabels, ax=self.ax)
		self.fig.canvas.draw()



class DoTest1:
	def __init__(self, domstack):
		self.ugsat=UgsaTraverseGraph(domstack, 1, 1.2, 1, 1, True, False)
		self.ugsat2=UgsaTraverseGraph(domstack, 1, 1.2, 1, 1, False, False)
		
	def runs(self, slast, sfirst=(0,0)):
		for s in [(i,j) for i in range(10) for j in range(10)]:
			
			if s < sfirst:
				continue
			if s > slast:
				break
			a = self.ugsat.doSearch(s)
			b = self.ugsat2.doSearch(s)
			print s, a, b
			assert(a == b)

