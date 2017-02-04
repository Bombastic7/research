 #!/bin/python


import random
import csv
import math
import json
import subprocess
 
 
 
def genGridNavMap(h, w, block, fname):
	
	with open(fname, "w") as f:
		for i in range(0, h*w):
			
			val = 0
			if random.uniform(0, 1) < block:
				val = 1
			
			f.write(str(val) + " ")
			
			if (i+1) % w == 0:
				f.write("\n")



def genGridNavProblemSet(fmapname, block, h, w, nprobs, mindist):
	
	#~ genGridNavMap(h, w, block, fmapname)
	
	#~ cellsRows = []
	#~ with open(fmapname, "rb") as f:
		#~ reader = csv.reader(f, delimiter=" ")
		
		#~ for row in reader:
			#~ cellsRows.append(row)

	proc = subprocess.Popen(["./gridnav/test_connected", fmapname], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	
	griddiag = math.hypot(h-1, w-1)
	
	probs = []
	
	def startGoalCond(startpos, endpos):
		if abs(math.hypot(*startpos) - math.hypot(*goalpos)) < griddiag * mindist:
			return False
		
		proc.stdin.write(str(startpos) + " " + str(endpos) + "\n")
		return proc.stdout.readline() == "true"


	
	for i in range(0, nprobs):
		startpos = (0,0)
		goalpos = (0,0)

		while not startGoalCond(startpos, goalpos):
			startpos = (random.randint(0, w-1), random.randint(0, w-1))
			goalpos = (random.randint(0, w-1), random.randint(0, w-1))
		
		probs.append({"init" : startpos, "goal" : goalpos, "map" : fmapname})
	
	proc.stdin.write("\n")
	proc.wait()
	
	return probs

		
