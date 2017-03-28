
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include "domain/gridnav/blocked/graph.hpp"
#include "domain/pancake/fwd.hpp"
#include "domain/tiles/fwd.hpp"
//#include "domain/star_abt.hpp"

#include "util/debug.hpp"
#include "util/json.hpp"

//#include "search/hastar/v2/hastar.hpp"

#include "search/solution.hpp"

#include "domain/test_graph.hpp"
#include "search/ugsa/v6/ugsa.hpp"

#include "search/admissible_abtsearch2.hpp"
#include "search/astar.hpp"
#include "search/bugsy.hpp"
#include "search/debug_walker.hpp"

#include "domain/gridnav/hypernav/hypernav_real.hpp"
#include "search/astar2.hpp"

#include "search/ugsa.hpp"

namespace mjon661 {

	
	std::vector<int> korf_15tiles_100instances(unsigned i) {
		fast_assert(i >= 1 && i <= 5);
		
		switch(i) {
			case 1:
				return {14,13,15,7,11,12,9,5,6,0,2,1,4,8,10,3};
			case 2:
				return {13,5,4,10,9,12,8,14,2,3,7,1,0,15,11,6};
			case 3:
				return {14,7,8,2,13,11,10,4,9,12,5,0,3,6,1,15};
			case 4:
				return {5,12,10,7,15,11,14,0,8,2,1,13,3,4,9,6};
			case 5:
				return {4,7,14,13,10,3,9,12,11,5,6,15,1,2,8,0};
		}
		
		return {};
	}
	
	
	
	
	
	//Insert results of Alg_t run on pDomStack into jRes_nonutil[pAlgName], 
	//	and also into jRes_util[weight][pAlgName_pseudo] with utility value added, for weight in pWeights.
	template<typename D, typename Alg_t>
	void run_nonutil_search_fixedexptime(	D& pDomStack,
											Json const& jAlgConfig, 
											Json& jRes_nonutil, 
											Json& jRes_util, 
											std::string pAlgName,
											std::vector<std::tuple<double,double,std::string>> pWeights,
											double pFixedExpTime)
	{
		
		Timer searchTimer;
		Alg_t alg(pDomStack, jAlgConfig);
		
		bool success = false;
			
		searchTimer.start();
		try {
			alg.execute(pDomStack.getInitState());
			searchTimer.stop();
			success = true;
		} catch(NoSolutionException const& e) {
			jRes_nonutil["failed"]["astar"] = e.what();
		}
		
		if(success) {
			jRes_nonutil["astar"] = alg.report();
			jRes_nonutil["astar"]["walltime"] = searchTimer.seconds();
			
			std::string pseudoNameStr = pAlgName + "_pseudo";
			
			for(auto& w : pWeights) {
				jRes_util[std::get<2>(w)][pseudoNameStr] = jRes_nonutil[pAlgName];
				jRes_util[std::get<2>(w)][pseudoNameStr]["utility"] = 
					jRes_util[std::get<2>(w)][pseudoNameStr]["goal_g"].get<double>() * std::get<0>(w)
					+
					jRes_util[std::get<2>(w)][pseudoNameStr]["expd"].get<double>() * std::get<1>(w) * pFixedExpTime;
			}
		}
	
	}
	
	
	
	//Insert results of Alg_t run on pDomStack, for each weight in pWeights, into jRes[weight][pAlgName].
	//jAlgConfig is used as base of actual algorithm config; a copy is made and wf/wt added from pWeights.
	template<typename D, typename Alg_t>
	void run_util_search_fixedexptime(	D& pDomStack,
										Json const& jAlgConfig_tmpl, 
										Json& jRes,
										std::string const& pAlgName,
										std::vector<std::tuple<double,double,std::string>> pWeights,
										double pFixedExpTime)
	{		
		Timer searchTimer;
		
		Json jAlgConfig = jAlgConfig_tmpl;
		fast_assert(pFixedExpTime == jAlgConfig_tmpl.at("fixed_exptime").get<double>());
		
		for(auto& weight : pWeights) {
			jAlgConfig["wf"] = std::get<0>(weight);
			jAlgConfig["wt"] = std::get<1>(weight);
		
			Alg_t alg(pDomStack, jAlgConfig);
		
			bool success = false;
		
			try {
				searchTimer.start();
				alg.execute(pDomStack.getInitState());
				searchTimer.stop();
				success = true;
			} catch(NoSolutionException const& e) {
				jRes[std::get<2>(weight)]["failed"][pAlgName] = e.what();
			}
		
			if(success) {
				jRes[std::get<2>(weight)][pAlgName] = alg.report();
				jRes[std::get<2>(weight)][pAlgName]["walltime"] = searchTimer.seconds();
				jRes[std::get<2>(weight)][pAlgName]["utility"] = 
					jRes[std::get<2>(weight)][pAlgName]["goal_g"].get<double>() * std::get<0>(weight) +
					jRes[std::get<2>(weight)][pAlgName]["expd"].get<double>() * pFixedExpTime * std::get<1>(weight);
			}
		}
	}
	
	
	
	
	
	
	template<typename D>
	void run_bugsy_rollingbf_allOptions(D& pDomStack,
										Json const& jAlgConfig_tmpl, 
										Json& jRes,
										std::string const& pAlgName,
										std::vector<std::tuple<double,double,std::string>> pWeights,
										double pFixedExpTime)
	{
	
	using namespace algorithm::bugsy;
	
	/*	
	bugsyRollingBfOptions = (
		('E_TgtProp', ['depth', 'f', 'uRound']),
		('E_KeepCounts', ['keepcounts', 'dropcounts']),
		('E_PruneOutliers', ['prune', 'nopr']),
		('E_Kfactor', ['none', 'openlistsz']),
		('E_EvalProp', ['dist', 'distAndDepth'])
	)
	
	opToks = [['C_RollingBf::'+op[0]+'::'+val for val in op[1]] for op in bugsyRollingBfOptions]
		
	algTypeTmpl = 'BugsyImpl<D,true,CompRemExp_rollingBf<D,{},{},{},{},{}>>;'
	funcCallStr = 'run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);'
		
	allOps = itertools.product(*opToks)
	
	with open("outpy.txt", "w") as f:
		for ops in allOps:
			print('{', file=f)
			print('using Alg_t=' + algTypeTmpl.format(*ops), file=f)
			print std::string algNameWithOpsStr = pAlgName +'_'+'C_RollingBf::niceNameStr('+','.join([i for i in ops])+')'
			print(funcCallStr, file=f)
			print('}', file=f)
	*/

{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::depth,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::f,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::keepcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::prune,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::none,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::dist);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}
{
using Alg_t=BugsyImpl<D,true,CompRemExp_rollingBf<D,C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth>>;
std::string algNameWithOpsStr = pAlgName +"_"+C_RollingBf::niceNameStr(C_RollingBf::E_TgtProp::uRound,C_RollingBf::E_KeepCounts::dropcounts,C_RollingBf::E_PruneOutliers::nopr,C_RollingBf::E_Kfactor::openlistsz,C_RollingBf::E_EvalProp::distAndDepth);
run_util_search_fixedexptime<D,Alg_t>(pDomStack, jAlgConfig_tmpl, jRes, algNameWithOpsStr, pWeights, pFixedExpTime);
}

	}
	

	
	
	
	Json run_tiles_44() {
		using D = tiles::TilesGeneric_DomainStack<4,4,true,false,1>;

		Json jDomConfig, jAlgConfig, jRes;
		
		
		jDomConfig["goal"] = std::vector<unsigned>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		jDomConfig["kept"] = std::vector<unsigned>{  1,1,1,1,1,1,1,1,1,1 ,1 ,1 ,1 ,1 ,1 };
		D domStack(jDomConfig);

		const double fixedExpTime = 3e-6;
		
		
		for(unsigned i=1; i<=5; i++) {
			tiles::BoardState<4,4> initState(korf_15tiles_100instances(i));
			domStack.setInitState(initState);
			
			std::string probKey = std::to_string(i);
			jRes[probKey] = Json();
			jRes.at(probKey)["nonutil"] = Json();
			jRes.at(probKey)["util"] = Json();
				
			std::vector<std::tuple<double,double,std::string>> weights = {
				std::tuple<double,double,std::string>{1,1e6,"1e6"},
				std::tuple<double,double,std::string>{1,1e3, "1e3"},
				std::tuple<double,double,std::string>{1,1,"1"}
			};
			
			jAlgConfig["fixed_exptime"] = 3e-6;
			jAlgConfig["expd_limit"] = 200e3;
			
			run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard>>(
				domStack, jAlgConfig, jRes.at(probKey).at("nonutil"), jRes.at(probKey).at("util"), "astar", weights, fixedExpTime);
			
			run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard>>(
				domStack, jAlgConfig, jRes.at(probKey).at("nonutil"), jRes.at(probKey).at("util"), "speedy", weights, fixedExpTime);
			
			run_nonutil_search_fixedexptime<D, algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Greedy>>(
				domStack, jAlgConfig, jRes.at(probKey).at("nonutil"), jRes.at(probKey).at("util"), "greedy", weights, fixedExpTime);
			
			
			run_util_search_fixedexptime<D, algorithm::bugsy::BugsyImpl<D, true, algorithm::bugsy::CompRemExp_delay<D>>>(
				domStack, jAlgConfig, jRes.at(probKey).at("util"), "greedy", weights, fixedExpTime);
			
			
			run_bugsy_rollingbf_allOptions(domStack, jAlgConfig, jRes.at(probKey).at("util"), "bugsy", weights, fixedExpTime);

		}
		return jRes;
	}

}

int main(int argc, const char* argv[]) {
	std::cout << mjon661::run_tiles_44().dump(4) << "\n";

}
