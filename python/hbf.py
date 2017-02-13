#!/bin/python

import misc, hastar
import numpy as np
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
		self.abtdom = dom.spawnAbtDomain()
		self.abtalg = hastar.HAstar(dom)


	#sets self.rec, which is an dict of solutioncost -> list of expansion counts.
	#Performs n searches, stores solution cost and nodes expanded.
	def execute_solcost(self, runs):
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

	
	#Executes n searches, returns list of dicts, one for each search, mapping f-level to expansions at that level.
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
		if hasattr(self.dom, "hval"):
			return self.dom.hval(s)
		return self.abtalg.execute(self.abtdom.abstractState(s), 1).f
		

	
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





#fcounts is list of dicts as produced by execute_countf.
#For each fcount dict, make list of f-level count values, ordered by f-level key. Return these lists in a list.
def flatten_fcounts(fcounts):
	flists = []
	
	for fc in fcounts:
		fl = [i for i in fc.iteritems()]
		fl.sort()
		flists.append([i[1] for i in fl])
	
	return flists


#flists is list of lists of expansions at each f-level, ordered by f-level.
#returns list of lists of computed hbf, for each pair of adjacent f-level counts in each input list.
#hbf(flvl) = count(flvl+1) / count(flvl)
def compute_hbf_A(flists):
	hbflists = []
	
	for i in range(len(flists)):
		s = []
		fl = flists[i]
		
		for i in range(len(fl)-2): #last f-level is ignored as it presumably wasn't fully explored.
			assert(i >= 0)
			s.append(float(fl[i+1]) / fl[i])

		hbflists.append(s)
	
	return hbflists


#Same as compute_hbf_A, but hbf(flvl) = (count(flvl+1) + C) / C, where C is sum of 
#	count(j) where j <= flvl.
def compute_hbf_B(flists):
	hbflists = []
	
	cumflists = [[j for j in i] for i in flists]
	
	for fl in cumflists:
		s = 0
		for i in range(len(fl)):
			fl[i] += s
			s = fl[i]

	for i in range(len(cumflists)):
		s = []
		fl = cumflists[i]
		
		for i in range(len(fl)-2):
			assert(i >= 0)
			s.append(float(fl[i+1]) / fl[i])

		hbflists.append(s)
	
	return hbflists


#Returns list of lists. The i-th list contains the i-th element of each list in hbflists (if long enough).
def arrangeRelativeFlevel(hbflists):
	hbflvl = []
	i=0
	lst2 = [l for l in hbflists]
	
	while True:
		lst2 = [l for l in lst2 if len(l) > i]
		
		if len(lst2) == 0:
			break
	
		x = []
		for l in lst2:
			x.append(l[i])

		hbflvl.append(x)
		i += 1
	
	return hbflvl


def meanOfMeans(lst):
	x = [(np.mean(i), np.std(i, ddof=1)) for i in lst]
	x = [(m,s) for (m,s) in x if not np.isnan(m) and not np.isnan(s)]
	return np.mean([i[0] for i in x]), np.mean([i[1] for i in x])


def medOfMeans(lst):
	x = [(np.mean(i), np.std(i, ddof=1)) for i in lst]
	x = [(m,s) for (m,s) in x if not np.isnan(m) and not np.isnan(s)]
	return np.median([i[0] for i in x]), np.median([i[1] for i in x])


def makeReport(dom):
	alg = InvestigateHBF(dom)
	fcounts = alg.execute_countf(50)
	flists = flatten_fcounts(fcounts)
	
	rep = {}
	
	hbflists_A = compute_hbf_A(flists)
	hbflists_B = compute_hbf_B(flists)
	
	hbflvl_A = arrangeRelativeFlevel(hbflists_A)
	hbflvl_B = arrangeRelativeFlevel(hbflists_B)
	
	rep["mean hbf_A"] = meanOfMeans(hbflists_A)
	rep["median hbf_A"] = medOfMeans(hbflists_A)
	rep["mean hbf_B"] = meanOfMeans(hbflists_B)
	rep["median hbf_B"] = medOfMeans(hbflists_B)
	rep["mean rel flvl hbf_A"] = meanOfMeans(hbflvl_A)
	rep["mean rel flvl hbf_B"] = meanOfMeans(hbflvl_B)
	
	rep["hbflvl_A"] = hbflvl_A
	rep["hbflvl_B"] = hbflvl_B
	rep["hbflists_A"] = hbflists_A
	rep["hbflists_B"] = hbflists_B
	return rep
