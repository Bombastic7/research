#!/bin/python


def _bstr(b):
	return "true" if b else "false"


def gridnav_blocked(height, width, mv8, cstLC, hr):
	
	declStr = "gridnav::blocked::GridNav_DomainStack_single<{0}, {1}, {2}, {3}, {4}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), _bstr(hr))
	
	return declStr
	
	
	


def gridnav_blocked_stack_merge(height, width, mv8, cstLC, hfact, wfact, fillfact, maxAbtLvl = 1000):
	
	declStr = "gridnav::blocked::GridNav_DomainStack_MergeAbt<{0},{1},{2},{3},{mxL},{4},{5},{6}>"\
	.format(height, width, _bstr(mv8), _bstr(cstLC), hfact, wfact, fillfact, mxL=maxAbtLvl);
	
	return declStr



def pancake_stack_single(Ncakes, gapH, name = None):
	
	declStr = "pancake::Pancake_DomainStack_single<{0},{1}>".format(Ncakes, hTag);

	return declStr
	


def pancake_stack_ignore(Ncakes, Abt1Sz, AbtStep, name = None):
	
	declStr = "pancake::Pancake_DomainStack_IgnoreAbt<{0},{1},{2}>".format(Ncakes, Abt1Sz, AbtStep);

	return declStr




def tiles_stack(height, width, weighted, useH, Abt1Sz, name = None):
	
	declStr = "tiles::TilesGeneric_DomainStack<{0}, {1}, {2}, {3}, {4}>".format(height, width, wTag, hTag, Abt1Sz)
	
	return declStr
