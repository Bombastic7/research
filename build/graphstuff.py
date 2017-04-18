from __future__ import print_function
import matplotlib.pyplot as plt
import math
import itertools
import json
import networkx as nx

graphNames = {}


def makeScatterPlot(grphname, y):
	fig, ax = plt.subplots()
	
	ax.scatter(range(len(y)), y)
	
	fig.suptitle(grphname)
	
	return fig, ax


def makeBFseries(y):
	assert(len(y) > 1)
	
	fcountdict = {}
	
	for i in y:
		if i not in fcountdict:
			fcountdict[i] = 0
		fcountdict[i] += 1
	
	fcountpairs = [i for i in fcountdict.iteritems()]
	fcountpairs.sort(cmp=lambda a,b: -1 if a[0] < b[0] else 1)
	
	bfseries = []
	
	for i in xrange(1, len(fcountpairs)):
		bfseries.append(float(fcountpairs[i][1]) / fcountpairs[i-1][1])
	
	return bfseries




def makeBFseries_cum(y):
	assert(len(y) > 1)
	
	fcountdict = {}
	
	for i in y:
		if i not in fcountdict:
			fcountdict[i] = 0
		fcountdict[i] += 1
	
	fcountpairs = [list(i) for i in fcountdict.iteritems()]
	fcountpairs.sort(cmp=lambda a,b: -1 if a[0] < b[0] else 1)
	
	acc = fcountpairs[0][1]
	for i in xrange(1, len(fcountpairs)):
		newacc = acc + fcountpairs[i][1]
		fcountpairs[i][1] += acc
		acc = newacc
	
	bfseries = []
	
	for i in xrange(1, len(fcountpairs)):
		bfseries.append(float(fcountpairs[i][1]) / fcountpairs[i-1][1])
	
	return bfseries




def makeExpSeries(explist):
	serieslist = []
	
	for i in range(0, len(explist), 400):
		d = {}
		for j in range(0, i):
			if explist[j] not in d:
				d[explist[j]] = 0
			d[explist[j]] += 1
		
		serieslist.append([])

		for cst, n in d.iteritems():
			serieslist[-1].append((cst,n))
		
		serieslist[-1].sort()
	
	return serieslist


def plotExpSeries(explist):
	serieslist = makeExpSeries(explist)
	
	fig, ax = plt.subplots()
	
	for s in serieslist:
		color = str(0.9 * (float(serieslist.index(s)) / len(serieslist)))
		x = [i[0] for i in s]
		y = [i[1] for i in s]
		
		ax.plot(x,y, color=color)
		
		
def plotExpSeriesBar(explist):
	
	for i in range(0, len(explist), 400):
		fig, ax = plt.subplots()
		fig.suptitle(str(i))
	
		ax.hist([math.log(i) for i in explist[0:i]])

def plotExpSeries2(explist):
	fig, ax = plt.subplots()
	ax.scatter([i for i in range(len(explist))], explist)


def makeCumSeries(explist):
	serieslist = []
	
	fig, ax = plt.subplots()
	
	for e in range(200, len(explist), 200):
		d = {}
		for j in range(0, e):
			if explist[j] not in d:
				d[explist[j]] = 0
			d[explist[j]] += 1
		
		serieslist = ([],[])
		
		cstcountpairs = [i for i in d.iteritems()]
		cstcountpairs.sort()
		
		plotcolor = str(float(e) / len(explist) * 0.9)
		
		ax.plot([i[0] for i in cstcountpairs], [i[1] for i in cstcountpairs], color=plotcolor)
	
	
	
	
	
def plotScatter(explist):
	fig,ax = plt.subplots()
	ax.scatter(range(len(explist)), explist)
	
	
	
	
	
	
	
	
def makeBugsyOptionPerms():
	
	tmplt = 'run_bugsy_fixed_rollingbf<D, {},{},{},{},{}>(domStack, jAlgConfig, algName, jRes);'
	
	opsets = itertools.product([0,1,2], [0,1], [0,1], [0,1], [0,1])
	
	return [tmplt.format(*i) for i in opsets]
	
	


def doBugsyResultRankings(rawdata):
	r = {}
	for probkey in rawdata.keys():
		r[probkey] = []
		for algkey in rawdata[probkey].keys():
			if(rawdata[probkey][algkey]['expd'] < 500e3):
				r[probkey].append((rawdata[probkey][algkey]['utility'], rawdata[probkey][algkey]['expd'], algkey))
			else:
				pass
				#r[probkey].append((float('Inf'), rawdata[probkey][algkey]['expd'], algkey))
		
		r[probkey].sort()
	
	return r




def testStuff():
	bugsyRollingBfOptions = (
		('E_TgtProp', ['depth', 'f', 'uRound']),
		('E_KeepCounts', ['keepcounts', 'dropcounts']),
		('E_PruneOutliers', ['prune', 'nopr']),
		('E_Kfactor', ['none', 'openlistsz']),
		('E_EvalProp', ['dist', 'distAndDepth']),
		('E_BfAvgMethod', ['bfArMean', 'bfGeoMean'])
	)

	opToks = [['C_RollingBf::'+op[0]+'::'+val for val in op[1]] for op in bugsyRollingBfOptions]
		
	algTypeTmpl = 'BugsyImpl<D,true,CompRemExp_rollingBf<D,{},{},{},{},{}>>;'
	funcCallStr = 'run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);'
		
	allOps = itertools.product(*opToks)
	
	with open("bugsy_rollingbf_options.inc", "w") as f:
		for ops in allOps:
			print('{', file=f)
			print('using Alg_t=' + algTypeTmpl.format(*ops), file=f)
			print("""std::string algNameWithOpsStr = pAlgName +'_'+'C_RollingBf::niceNameStr("""+','.join([i for i in ops])+')', file=f)
			print(funcCallStr, file=f)
			print('}', file=f)



def analysisTileInitStates():
	with open("out.txt") as f:
		rawdata = json.load(f)
	
	histoData = {}
	
	for domkey in rawdata:
		hg = {}
		hd = {}
		for probkey in rawdata[domkey]:
			i = rawdata[domkey][probkey][1]
			if i not in hg:
				hg[i] = 0
			hg[i] += 1
		
			i = rawdata[domkey][probkey][2]
			if i not in hd:
				hd[i] = 0
			hd[i] += 1

		histoData[domkey] = {"hg": hg, "hd": hd}
	
	return histoData




def reduceRunLength(s):
	r = []
	for i in s:
		if len(r) == 0 or r[-1][0] != i:
			r.append([i, 1])
		else:
			r[-1][1] += 1
	return r


def reduceRunLength2(a,b):
	r = []
	assert(len(a) == len(b))
	
	for i in range(len(a)):
		if len(r) == 0:
			r.append([(a[i], b[i]), 1])
	
		elif a[i] == r[-1][0][0] or b[i] == r[-1][0][1]:
			r[-1][1] += 1
		
		else:
			r.append([(a[i],b[i]), 1])
	
	return r
	
	
	for i in range(len(a)):
		r.append((a,b))
	




class GridnavLifeCostGraph:
	def __init__(self, w, h):
		self.height = h
		self.width = w
		self.s = (0,0)
		self.g = (0,0)
		self.fig, self.ax = plt.subplots()
		
		self.graph = nx.DiGraph()
		
		for h in range(self.height):
			for w in range(self.width):
				self.graph.add_node((w,h))
		
		for h in range(self.height):
			for w in range(self.width):
				if(h != 0):
					self.graph.add_edge((w,h), (w,h-1), weight=h)
				if(h != self.height-1):
					self.graph.add_edge((w,h), (w,h+1), weight=h)
				if(w != 0):
					self.graph.add_edge((w,h), (w-1,h), weight=h)
				if(w != self.width-1):
					self.graph.add_edge((w,h), (w+1,h), weight=h)
				
				if(h != 0 and w != 0):
					self.graph.add_edge((w,h), (w-1,h-1), weight=h*math.sqrt(2))
				if(h != 0 and w != self.width-1):
					self.graph.add_edge((w,h), (w+1,h-1), weight=h*math.sqrt(2))
				if(h != self.height-1 and w != 0):
					self.graph.add_edge((w,h), (w-1,h+1), weight=h*math.sqrt(2))
				if(h != self.height-1 and w != self.width-1):
					self.graph.add_edge((w,h), (w+1,h+1), weight=h*math.sqrt(2))
	
	
	def drawGraph(self):
		optpath = nx.dijkstra_path(self.graph, self.s, self.g)
		pos = {}
		for n in self.graph.nodes():
			pos[n] = n
		
		color = []
		for n in self.graph.nodes():
			if n == self.s:
				color.append('g')
			elif n == self.g:
				color.append('m')
			elif n in optpath:
				color.append('b')
			else:
				color.append('r')

		nx.draw_networkx_nodes(self.graph, pos=pos, node_color=color, ax=self.ax)
	
	def up(self):
		self.g = (self.g[0], max(self.g[1]-1, 0))
		self.drawGraph()
	def down(self):
		self.g = (self.g[0], min(self.g[1]+1, self.height-1))
		self.drawGraph()
	def left(self):
		self.g = (max(self.g[0]-1, 0), self.g[1])
		self.drawGraph()
	def right(self):
		self.g = (min(self.g[0]+1, self.width-1), self.g[1])
		self.drawGraph()
	
	def nextPos(self):
		if self.g[0] != 0 and self.g[0] != self.width-1 and self.g[1] != 0 and self.g[1] != self.height-1:
			self.up()
		elif self.g == (0,0):
			self.g = (0,1)
		elif self.g == (0,self.height-1):
			self.g = (1,self.height-1)
		elif self.g == (self.width-1, self.height-1):
			self.g = (self.width-1, self.height-2)
		elif self.g == (self.width-1, 0):
			self.g = (self.width-2, 0)
		
		elif self.g[1] == 0:
			self.left()
		elif self.g[1] == self.height-1:
			self.right()
		elif self.g[0] == 0:
			self.down()
		elif self.g[0] == self.width-1:
			self.up()
		
		self.drawGraph()

	def vpf(self, s, g):
		acc = 0
		if s <= g:
			for i in range(s,g):
				acc += i
		else:
			for i in range(g+1, s+1):
				acc += i
		return acc
	
	
	
	#~ def computeOptPath(self):
		#~ optpath = []
		#~ d = math.sqrt(2)
		
		#~ mvs = ((0,-1,1), (0,1,1), (-1,0,1), (1,0,1),
				#~ (-1,-1,d), (1,-1,d), (-1,1,d), (1,1,d))
		
		#~ dist = {}
		
		#~ for i in range(self.sz):
			#~ p = (i % self.width, i / self.height)
			#~ dist[p] = 1e9
			
		#~ dist[self.s] = 0
		
		#~ while True:
			#~ bstcst = 1e9
			#~ for p, cst in dist.iteritems():
				#~ if bstcst > cst:
					#~ bstcst = cst
					#~ bstpos = p
			
			#~ for mv in mvs:
				#~ adjpos = (bstpos
			
		



def plot_graph1(rawdata, weightkey, domkey, tgtElm):
	fig, ax = plt.subplots()
	
	algkeys = rawdata.keys()
	algkeys.sort()
	
	probkeys = rawdata.values()[0].values()[0][domkey].keys()
	probkeys.sort()
	
	left = 0
	
	for ak in algkeys:
		probseries = []
		for pk in probkeys:
			probseries.append(rawdata[ak][weightkey][domkey][pk][tgtElm])
		
		barleft = [i+left for i in range(len(probseries))]
	
		ax.bar(barleft, probseries, 0.8)
		
		left += 2 * len(probseries)
	
	
	ax.set_xticks([i*2*len(probseries) for i in range(len(algkeys))])
	ax.set_xticklabels(algkeys)
	
	fig.suptitle(domkey + ", " + weightkey + ": " + "problem / " + tgtElm)
	return fig, ax
