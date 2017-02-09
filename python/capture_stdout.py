#!/bin/python

import sys
import time


objinst = None


class StdOutInt:
	def __init__(self, outfile):
		self.out = open(outfile, "w")
		self.origstdout = sys.stdout
		sys.stdout = self

	def write(self, s):
		self.out.write(s)
		self.out.flush()
		self.origstdout.write(s)
	
	def __getattr__(self, name):
		return self.origstdout.__getattr__(name)

	def stop(self):
		self.out.close()
		sys.stdout = self.origstdout


def start(outfile = str(int(time.time()))):
	global objinst
	assert(objinst is None)
	objinst = StdOutInt(outfile)


def stop():
	global objinst
	assert(objinst is not None)	
	objinst.stop()
	objinst = None
