from __future__ import print_function
import itertools

def makeBugsyRollingBfInc():
	bugsyRollingBfOptions = (
		('E_TgtProp', ['depth', 'f', 'uRound']),
		('E_KeepCounts', ['keepcounts', 'dropcounts']),
		('E_PruneOutliers', ['prune', 'nopr']),
		('E_Kfactor', ['none', 'openlistsz']),
		('E_EvalProp', ['dist', 'distAndDepth']),
		('E_BfAvgMethod', ['bfArMean', 'bfGeoMean'])
	)

	opToks = [['C_RollingBf::'+op[0]+'::'+val for val in op[1]] for op in bugsyRollingBfOptions]
		
	algTypeTmpl = 'BugsyImpl<D,true,CompRemExp_rollingBf<D,{},{},{},{},{},{}>>;'
	funcCallStr = 'run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);'
		
	allOps = itertools.product(*opToks)
	
	with open("bugsy_rollingbf_options.inc", "w") as f:
		for ops in allOps:
			print('{', file=f)
			print('using Alg_t=' + algTypeTmpl.format(*ops), file=f)
			print("""std::string algNameWithOpsStr = pAlgName + "_" + C_RollingBf::niceNameStr("""+','.join([i for i in ops])+');', file=f)
			print(funcCallStr, file=f)
			print('}', file=f)
