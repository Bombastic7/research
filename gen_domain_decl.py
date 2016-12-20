



def gridnav_blocked(name = None, height, width, mv8, cstLC, hr):
	mvTag = "8" if mv8 else "4"
	costTag = "LC" if cstLC else "Unit"
	hTag = "H" if hr else "NoH"
	
	defaultDeclName = "GridNav_h{0}w{1}_{2}_{3}_{4}".format(height, width, mvTag, costTag, hTag)
	
	
	
	if not name:
		name = defaultDeclName
	
	declStr = "using " + name + " = gridnav::blocked::GridNav_DomainStack_single<{0}, {1}, \
	typename gridnav::blocked::GridNavBase<{2}, {3}, {4}>::type >;"\
	.format(height, width, mvTag, costTag, hTag)
	
	return declStr
	
	
	


def gridnav_blocked_stack_merge(name = None, height, width, mv8, cstLC, hr, hfact, wfact, fillfact, maxAbtLvl = 1000):
	mvTag = "8" if params[2] else "4"
	costTag = "LC" if params[3] else "Unit"
	
	defaultDeclName = "GridNav_h{0}w{1}_{2}_{3}_MergeAbt_{4}_{5}_{6}".format(height, width, mv8, cstLC, hfact, wfact, fillfact)
	
	
	if not name:
		name = defaultDeclName
	
	
	declStr = "using " + name + " = gridnav::blocked::GridNav_DomainStack_MergeAbt<{0},{1},{2},{3},{mxL},{4},{5},{6}>;"\
	.format(height, width, mv8, cstLC, hfact, wfact, fillfact, mxL=MaxAbtLvl);
	
	return declStr



def pancake_stack(name = None, Ncakes, abtSz1, gapH):
	
	hTag = "H" if gapH else "NoH"
	
	baseTag = "Domain_"
	
	defaultDeclName = "Pancake_{0}_{1}_{2}".format(Ncakes, abtSz1, hTag)
	
	if not name:
		name = defaultDeclName
	
	declStr = "using " + name + " = 