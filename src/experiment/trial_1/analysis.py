#!/bin/python


import sys
import os
import json



VALUE_NAMES = ("solution length", "solution cost", "utility", "walltime")




def getValues(res):
	if res["status"] not "SUCCESS":
		return
	
	if res["solution length"] == 0:
		return
	
	(res[
