#!/bin/python


def _boolStr(b):
	return "true" if b else "false"


def gridnav_blocked(height, width, mv8, cstLC, hr, name = None):
	mvTag = "8" if mv8 else "4"
	costTag = "LC" if cstLC else "Unit"
	hTag = "H" if hr else "NoH"
	
	#defaultDeclName = "GridNav_h{0}w{1}_{2}_{3}_{4}".format(height, width, mvTag, costTag, hTag)
	
	
	
	#if not name:
	#	name = defaultDeclName
	
	declStr = "gridnav::blocked::GridNav_DomainStack_single<{0}, {1}, gridnav::blocked::GridNavBase<{2}, {3}, {4}>::type>"\
	.format(height, width, _boolStr(mv8), _boolStr(cstLC), _boolStr(hr))
	
	return declStr
	
	
	


def gridnav_blocked_stack_merge(height, width, mv8, cstLC, hr, hfact, wfact, fillfact, maxAbtLvl = 1000, name = None):
	mvTag = "8" if params[2] else "4"
	costTag = "LC" if params[3] else "Unit"
	
	defaultDeclName = "GridNav_h{0}w{1}_{2}_{3}_MergeAbt_{4}_{5}_{6}".format(height, width, mv8, cstLC, hfact, wfact, fillfact)
	
	
	if not name:
		name = defaultDeclName
	
	
	declStr = "gridnav::blocked::GridNav_DomainStack_MergeAbt<{0},{1},{2},{3},{mxL},{4},{5},{6}>"\
	.format(height, width, mv8, cstLC, hfact, wfact, fillfact, mxL=MaxAbtLvl);
	
	return declStr



def pancake_stack_single(Ncakes, gapH, name = None):
	
	hTag = "H" if gapH else "NoH"
	
	defaultDeclName = "Pancake_{0}_{1}".format(Ncakes, hTag);
	
	if not name:
		name = defaultDeclName
	
	declStr = "pancake::Pancake_DomainStack_single<{0},{1}>".format(Ncakes, hTag);

	return declStr
	


def pancake_stack_ignore(Ncakes, Abt1Sz, AbtStep, name = None):
	
	defaultDeclName = "Pancake_{0}_Abt_{1}_{2}".format(Ncakes, Abt1Sz, AbtStep);

	if not name:
		name = defaultDeclName
	
	declStr = "pancake::Pancake_DomainStack_IgnoreAbt<{0},{1},{2}>".format(Ncakes, Abt1Sz, AbtStep);

	return declStr




def tiles_stack(height, width, weighted, useH, Abt1Sz, name = None):
	
	wTag = "W" if weighted else "NoW"
	hTag = "H" if useH else "NoH"
	
	defaultDeclName = "Tiles_{0}_{1}_{2}_Abt_{3}".format(height*width-1, wTag, hTag, Abt1Sz)
	
	if not name:
		name = defaultDeclName
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, wTag, hTag, Abt1Sz)
	
	return declStr
