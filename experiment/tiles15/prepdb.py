#!/bin/python

import json, sqlite3




def prepDB(resfile, dbfile):
	with open(resfile) as f:
		resdata = json.load(f)
	
	conn = sqlite3.connect(dbfile)
	
	c = conn.cursor()


	c.execute("CREATE TABLE hr_algorithms (algorithm text, hrspec text, problem text, weight_wf real, weight_wt real, \
				goal_g real, goal_depth integer, utility real, expd integer, gend integer, dups integer, reopnd integer)")
	
	for probkey in resdata.keys():
		for algkey in resdata[probkey]["nonutil"]:
			if algkey == "failed":
				continue
			
			hrspec = "domainhr"
			weight_wf = resdata[probkey]["util"][algkey]["wf"]
			weight_wt = resdata[probkey]["util"][algkey]["wt"]
			goal_g = resdata[probkey]["util"][algkey]["goal_g"]
			goal_depth = resdata[probkey]["util"][algkey]["goal_depth"]
			utility = resdata[probkey]["util"][algkey]["utility"]
			expd = resdata[probkey]["util"][algkey]["expd"]
			gend = resdata[probkey]["util"][algkey]["gend"]
			dups = resdata[probkey]["util"][algkey]["dups"]
			reopnd = resdata[probkey]["util"][algkey]["reopnd"]

		c.execute("INSERT INTO simple_algorithms VALUES (?,?,?,?,?,?,?,?,?,?,?,?)", 
			algkey, hrspec, probkey, weight_wf, weight_wt, goal_g, goal_depth, utility, expd, gend, dups, reopnd)



	c.execute("CREATE TABLE abt_algorithms (algorithm text, abtspec text, problem text, weight_wf real, weight_wt real, \
				goal_g real, goal_depth integer, expd integer, gend integer, dups integer, reopnd integer)")
