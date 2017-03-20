
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <utility>
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

#include "domain/gridnav/blocked/hypernav.hpp"
#include "domain/gridnav/blocked/test_hypernav.hpp"
#include "search/astar2.hpp"

#include "domain/gridnav/blocked/cellmap_real.hpp"

namespace mjon661 {
	
	/*
	template<typename D>
	void run_session1_minvals(	D& domStack, 
								typename D::template Domain<0>::State const& s0, 
								std::pair<double, double>& mincost, 
								std::pair<double, double>& mindist) {
		
		using MinCost_t = algorithm::AdmissibleAbtSearch2<D, 0, D::Top_Abstract_Level+1, true>;
		using MinDist_t = algorithm::AdmissibleAbtSearch2<D, 0, D::Top_Abstract_Level+1, false>;
		
		MinCost_t mincostalg(domStack, Json());
		MinDist_t mindistalg(domStack, Json());
		
		typename D::template Domain<0>::Cost h;
		unsigned d;
		
		mincostalg.doSearch(s0, h, d);
		mincost.first = h;
		mincost.second = d;
		
		mindistalg.doSearch(s0, h, d);
		mindist.first = h;
		mindist.second = d;
	}
	
	
	
	
	template<typename D>
	Json run_session1_uninformed(D& domStack, typename D::template Domain<0>::State const& s0) {
		
		using Alg_t = algorithm::AstarImpl<D, algorithm::AstarSearchMode::Uninformed, true, false>;
		
		Alg_t alg(domStack, Json());
		
		Solution<D> sol;
		alg.execute(s0, sol);
		
		return alg.report();
	}
	
	
	template<typename D>
	Json run_session1_astar(D& domStack, typename D::template Domain<0>::State const& s0) {
		
		using Alg_t = algorithm::AstarImpl<D, algorithm::AstarSearchMode::Standard, true, false>;
		
		Alg_t alg(domStack, Json());
		
		Solution<D> sol;
		alg.execute(s0, sol);
		
		Json res = alg.report();
		res["exp_f"] = alg.mTest_exp_f;
		return res;
	}
	
	
	template<typename D>
	Json run_session1_bugsy(D& domStack, typename D::template Domain<0>::State const& s0, double wf, double wt) {
		
		using Alg_min_t = algorithm::BugsyImpl<D, false, algorithm::BugsyHrMode::Abt_min>;
		using Alg_delayweight_t = algorithm::BugsyImpl<D, false, algorithm::BugsyHrMode::Abt_delayweight>;
		
		Json jConfig;
		jConfig["wf"] = wf;
		jConfig["wt"] = wt;
		
		Alg_min_t alg1(domStack, jConfig);
		Alg_delayweight_t alg2(domStack, jConfig);
		
		Solution<D> sol;
		alg1.execute(s0, sol);
		alg2.execute(s0, sol);
		
		Json res;
		res["hr_abt_min"] = alg1.report();
		res["hr_abt_delay_weight"] = alg2.report();
		return res;
	}
	*/
	/*
	template<typename D>
	Json run_session1_bugsy_bfmod(D& domStack, typename D::template Domain<0>::State const& s0, double wf, double wt, double bfmod) {
		
		using Alg_t = algorithm::Bugsy<D, false, true, true>;
		
		Json jConfig;
		jConfig["wf"] = wf;
		jConfig["wt"] = wt;

		Alg_t alg(domStack, jConfig, bfmod);
		
		Solution<D> sol;
		alg.execute(s0, sol);
		
		return alg.report();
	}
	
	
	template<typename D>
	Json run_session1_ugsa_bf_locked(D& domStack, typename D::template Domain<0>::State const& s0, double wf, double wt, double bf) {
		
		using Alg_t = algorithm::ugsav6::UGSAv6<D, false, algorithm::ugsav6::BFMode::Locked>;
		
		Json jConfig;
		jConfig["wf"] = wf;
		jConfig["wt"] = wt;

		Alg_t alg(domStack, jConfig, bf);
		
		Solution<D> sol;
		alg.execute(s0, sol);
		
		return alg.report();
	}
	
	
	
	
	Json run_session1_weightedAlgs(D& domStack, typename D::template Domain<0>::State const& s0) {
		
		std::vector<std::pair<double, double>> weights = {{1,1}, {10, 1}, {100, 1}};
		
		std::vector<double> bfvals = {1.01, 1.02, 1.03. 1.04, 1.1, 1,2};
		
		
		Json res;
		
		for(auto w : weights) {
			weightstr = std::to_string(w[0]) + "," + std::to_string(w[1]);
			
			res[weightstr]["bugsy"] = run_session1_bugsy(domStack, s0, w[0], w[1]);
			
			for(auto bf : bfvals) {
				bfstr = std::to_string(bf);
				
				res[weightstr]["bugsy_bfmod"][bfstr] = run_session1_bugsy_bfmod(domStack, s0, w[0], w[1], bf);
				
				res[weightstr]["ugsa_locked"][bfstr] = run_session1_ugsa_bf_locked(domstack, s0, w[0], w[1], bf);
			}
		}
		
		return res;
	}
	
	Json run_session1_stdAlgs(D& domStack, typename D::template Domain<0>::State const& s0) {
		
		Json res;
		
		res["astar"] = run_session1_astar(domStack, s0);
		res["uninformed"] = run_session1_uninformed(domStack, s0);
		return res;
	}
	*/
	
	/*
	Json run_session1_gridnav() {
		
		using D_1000_4_u = gridnav::blocked::GridNav_StarAbtStack<gridnav::blocked::CellGraph_4_hr<false,false>,10>;
		//~ using D_1000_4_w = gridnav::GridNav_StarAbtStack<gridnav::blocked::CellGraph_4_hr<true,false>,10>;
		//~ using D_2000_4_u = gridnav::GridNav_StarAbtStack<gridnav::blocked::CellGraph_4_hr<false,false>,10>;
		//~ using D_2000_4_w = gridnav::GridNav_StarAbtStack<gridnav::blocked::CellGraph_4_hr<true,false>,10>;
		
		//~ using D_1000_8_u = gridnav::GridNav_StarAbtStack<gridnav::blocked::CellGraph_8_hr<false,false>,10>;
		//~ using D_1000_8_w = gridnav::GridNav_StarAbtStack<gridnav::blocked::CellGraph_8_hr<true,false>,10>;
		//~ using D_2000_8_u = gridnav::GridNav_StarAbtStack<gridnav::blocked::CellGraph_8_hr<false,false>,10>;
		//~ using D_2000_8_w = gridnav::GridNav_StarAbtStack<gridnav::blocked::CellGraph_8_hr<true,false>,10>;
		
		Json jConfig;
		Json res;
		
		jConfig["height"] = 1000;
		jConfig["width"] = 1000;
		jConfig["map"] = ".1";
		
		for(int prob = -1; prob >= -3; prob--) {
			jConfig["init"] = prob;
			jConfig["goal"] = prob;
			
			D_1000_4_u d_1000_4_u(jConfig);
			
			res["1000_4_u"][std::to_string(prob)]["astar"] = run_session1_astar(d_1000_4_u, d_1000_4_u.getInitState());
			//res["1000_4_u"][std::to_string(prob)]["bugsy"]["1,0"] = run_session1_bugsy(d_1000_4_u, d_1000_4_u.getInitState(), 1, 0);
			//res["1000_4_u"][std::to_string(prob)]["bugsy"]["1,1"] = run_session1_bugsy(d_1000_4_u, d_1000_4_u.getInitState(), 1, 1);
			//res["1000_4_u"][std::to_string(prob)]["bugsy"]["10,1"] = run_session1_bugsy(d_1000_4_u, d_1000_4_u.getInitState(), 10, 1);
			//res["1000_4_u"][std::to_string(prob)]["bugsy"]["1,10"] = run_session1_bugsy(d_1000_4_u, d_1000_4_u.getInitState(), 1, 10);
		}
		
		return res;
	}
	*/
	//> 0.6 0 [ 91 50 33 ] [ 17 5 36 ]
	Json test_hypernav2() {
		
		using D_2 = gridnav::hypernav_blocked::TestDomainStack<2,1>;
		using Astar2_t = algorithm::Astar2Impl<D_2, algorithm::Astar2SearchMode::Uninformed>;
		
		Json jConfig;
		
		jConfig["map"] = std::string(",portalsSpanningTree,2000,2000,10");
		jConfig["dimsz"] = std::vector<unsigned>{2000,2000};
		
		D_2 domStack(jConfig);
		typename D_2::template Domain<0> dom(domStack);
		//using State = typename decltype(dom)::State;
		
		//domStack.assignInitGoalStates(std::pair<State, State>({0,0},{99,99}));


		
		
		
		std::ofstream dumpCellMapOfs("dump.txt");
		
		fast_assert(dumpCellMapOfs);
		
		for(unsigned h=0; h<2000; h++) {
			for(unsigned w=0; w<2000; w++) {
				if(domStack.mCellMap.isOpen(w+h*2000))
					dumpCellMapOfs << "O";
				else
					dumpCellMapOfs << " ";
			}
			dumpCellMapOfs << "\n";
		}
		
		dumpCellMapOfs.flush();
		
		Astar2_t astaralg(domStack, Json());
		
		astaralg.execute(domStack.getInitState());
		
		return astaralg.report();
		
	}
	
	
	Json test_hypernav() {
		using D_3 = gridnav::hypernav_blocked::TestDomainStack<3,1>;
		using Astar2_t = algorithm::Astar2Impl<D_3, algorithm::Astar2SearchMode::Standard>;
		
		Json res;
		
		for(auto blockedprob : {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9}) {
			Json jConfig;
			jConfig["map"] = std::string(",random,1,") + std::to_string(blockedprob);
			jConfig["dimsz"] = std::vector<unsigned>{100,99,100};
		
			D_3 domStack(jConfig);
			typename D_3::template Domain<0> dom(domStack);
		
			//~ dom.prettyPrintState(domStack.getInitState(), std::cout);
			//~ std::cout << "  ";
			//~ dom.prettyPrintState(domStack.mGoalState, std::cout);
			//~ std::cout << "\n";
			
			
		
			using State = typename decltype(dom)::State;
			
			std::vector<std::pair<State, State>> genInitGoal;
			
			double mindist = 86; // sqrt(100**2 + 99**2 + 100**2) / 2
			
			for(unsigned i=0; i<3; i++) {
				std::pair<State, State> sp;
				
				try {
					sp = domStack.genRandInitGoal(mindist);
				} catch(std::exception&) {
					res[jConfig["map"].get_ref<std::string&>()][std::to_string(i)] = "Failed to generate states.";
					continue;
				}
					
				domStack.assignInitGoalStates(sp);
				Astar2_t astaralg(domStack, Json());
				
				try {
					astaralg.execute(domStack.getInitState());
				}
				catch(NoSolutionException const&) {
					std::cout << "> " << blockedprob << " " << i << " ";
					dom.prettyPrintState(sp.first, std::cout);
					std::cout << " ";
					dom.prettyPrintState(sp.second, std::cout);
					std::cout << "\n";
				}
				res[jConfig["map"].get_ref<std::string&>()][std::to_string(i)] = astaralg.report();
			}
			
		}
	
		return res;
	}

	/*
	void run_ugsapure() {
		Json jDomConfig;
		//jDomConfig["goal"] = std::vector<unsigned>{0, 1, 2, 3, 4, 5, 6, 7, 8};
		//jDomConfig["kept"] = std::vector<unsigned>{5, 4, 3, 2, 1, 1, 1, 1};
		jDomConfig["map"] = ".1";
		jDomConfig["width"] = 100;
		jDomConfig["height"] = 100;
		
		using DomStack_t = gridnav::blocked::GridNav_StarAbtStack<gridnav::blocked::CellGraph_4_hr<false,false>,4>;
		
		DomStack_t domStack(jDomConfig);

		//auto s0 = domStack.getInitState();

		Solution<DomStack_t> sol;

		using UGSAAlg_t = algorithm::ugsav6::UGSAv6<DomStack_t, false>;
		//using AstarAlg_t = algorithm::AstarImpl<DomStack_t, algorithm::AstarSearchMode::Standard, true, true>;
		//using BugsyAlg_t = algorithm::BugsyImpl<DomStack_t, false, true, false>;
		//using BugsyAlg_bfmod_t = algorithm::BugsyImpl<DomStack_t, false, true, true>;

		Json jConfig;
		jConfig["wf"] = 1;
		jConfig["wt"] = 1;
		
		UGSAAlg_t ugsaAlg(domStack, jConfig, 1.01);
		//AstarAlg_t astaralg(domStack, jConfig);
		//BugsyAlg_t bugsyalg(domStack, jConfig);
		//BugsyAlg_bfmod_t bugsyalg_bfmod(domStack, jConfig, 1.02);
		
		ugsaAlg.execute(domStack.getInitState(), sol);
		//astaralg.execute(domStack.getInitState(), sol);
		//bugsyalg.execute(domStack.getInitState(), sol);
		//bugsyalg_bfmod.execute(domStack.getInitState(), sol);

		std::cout << "ugsa\n" << ugsaAlg.report().dump(4) << "\n\n\n\n";
		//std::cout << "astar\n" << astaralg.report().dump(4) << "\n\n\n\n";
		//std::cout << "bugsy\n" << bugsyalg.report().dump(4) << "\n\n\n\n";
		//std::cout << "bugsy_bfmod\n" << bugsyalg_bfmod.report().dump(4) << "\n\n\n\n";
		
		Json jExpU;
		//jExpU["bugsy"]["f"] = bugsyalg.mTest_exp_f;
		//jExpU["bugsy"]["u"] = bugsyalg.mTest_exp_u;
		//jExpU["bugsy_bfmod"]["u"] = bugsyalg.mTest_exp_u;
		//jExpU["bugsy_bfmod"]["f"] = bugsyalg.mTest_exp_f;
		//jExpU["astar"]["f"] = astaralg.mTest_exp_f;
		jExpU["ugsa"]["f"] = ugsaAlg.mAlgo.mTest_exp_f;
		jExpU["ugsa"]["u"] = ugsaAlg.mAlgo.mTest_exp_u;
		jExpU["ugsa"]["uh"] = ugsaAlg.mAlgo.mTest_exp_uh;
		
		{
			std::ofstream ofs("test_exp_u");
			fast_assert(ofs);
			ofs << jExpU.dump(4);
		}
		
		
		for(double maxbf = 1.0; maxbf <= 1.5; maxbf += 0.01) {
			Json jConfig;
			jConfig["wf"] = 1;
			jConfig["wt"] = 1;
			UGSAAlg_t ugsaAlg(domStack, jConfig, maxbf);
			

			ugsaAlg.execute(domStack.getInitState(), sol);

			Json jStats = ugsaAlg.report();
			std::cout << "Max BF: " << maxbf << "\n";
			std::cout << jStats.dump(4) << "\n\n\n\n";
		}
		
		
		
		using AstarAlg_t = algorithm::AstarImpl<DomStack_t, algorithm::AstarSearchMode::Standard>;
		
		AstarAlg_t astaralg(domStack, jConfig);
		
		astaralg.execute(domStack.getInitState(), sol);
		
		jStats = astaralg.report();
		std::cout << jStats.dump(4) << "\n\n\n\n";
		
		
		using BugsyAlg_t = algorithm::BugsyImpl<DomStack_t, false, true>;
		BugsyAlg_t bugsyalg(domStack, jConfig);
		
		bugsyalg.execute(domStack.getInitState(), sol);
		
		jStats = bugsyalg.report();
		std::cout << jStats.dump(4) << "\n\n\n\n";
		
		
		Json jExpU;
		jExpU["bugsy"]["f"] = bugsyalg.mTest_exp_f;
		jExpU["bugsy"]["u"] = bugsyalg.mTest_exp_u;
		jExpU["ugsa"]["f"] = ugsaAlg.mAlgo.mTest_exp_f;
		jExpU["ugsa"]["u"] = ugsaAlg.mAlgo.mTest_exp_u;
		
		{
			std::ofstream ofs("test_exp_u");
			fast_assert(ofs);
			ofs << jExpU.dump(4);
		}
		
	}
	*/
	
	
	Json test_pancake() {
		
		Json jDomConfig;
		jDomConfig["init"] = {8, 9, 4 ,3 ,7 ,0, 5, 2, 1, 6};
		jDomConfig["kept"] = {1, 1, 1, 1, 1, 2, 3, 4, 5, 6};
		
		using DomStack_t = pancake::Pancake_DomainStack_IgnoreAbt<10, 5, 1, false, false>;
		
		DomStack_t domStack(jDomConfig);
		typename DomStack_t::template Domain<0> dom(domStack);
		
		algorithm::Astar2Impl<DomStack_t, algorithm::Astar2SearchMode::Uninformed> astaralg(domStack, Json());
		
		astaralg.execute(domStack.getInitState());
		
		for(auto* n = astaralg.mGoalNode; n; n=n->parent) {
			decltype(dom)::State s;
			dom.unpackState(s, n->pkd);
			
			dom.prettyPrintState(s, std::cout);
			std::cout << "\n";
		}
		
		return astaralg.report();		
	}
	
	
	
	
	
	template<int H, int W, template<typename> typename Alg_t_D>
	Json run_tiles_hr(unsigned pInitSeed, Json const& jAlgConfig = Json()) {
		
		Json jDomConfig;
		//jDomConfig["goal"] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
		//jDomConfig["kept"] = {   5, 4, 3, 2, 1, 1, 1, 1};
		
		//jDomConfig["goal"] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		//jDomConfig["kept"] = {   7, 6, 5, 4, 3, 2, 1, 1, 1, 1,  1};
		
		//jDomConfig["goal"] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
		//jDomConfig["kept"] = {   8, 7, 6, 5, 4, 3, 2, 1, 1, 1,  1,  1,  1,  1,  1};
		
		std::vector<int> goalState, tilesDropLevel;
		std::string initStateStr = "[";
		
		for(unsigned i=0; i<H*W; i++) {
			goalState.push_back(i);
		}
		
		unsigned abt1droptiles = (H*W-1) / 2;
		tilesDropLevel.resize(H*W-1);
		
		for(unsigned i=H*W-2-abt1droptiles; i<H*W-1; i++) {
			tilesDropLevel.at(i) = 1;
		}
		
		for(unsigned i=H*W-2-abt1droptiles, j=2; i!=(unsigned)-1; i--, j++) {
			tilesDropLevel.at(i) = j;
		}
		
		jDomConfig["goal"] = goalState;
		jDomConfig["kept"] = tilesDropLevel;
		
		

		using DomStack_t = tiles::TilesGeneric_DomainStack<H, W, true, true, 1>;
		using Alg_t = Alg_t_D<DomStack_t>;
		//using Alg_t = algorithm::DebugWalker<DomStack_t>;
		
		
		DomStack_t domStack(jDomConfig);
		domStack.setInitState(domStack.randInitState(pInitSeed));
		
		{
			auto initState = domStack.getInitState();
			
			for(unsigned i=0; i<H*W; i++)
				initStateStr += std::to_string(initState[i]) + ",";
			
			initStateStr += "]";
		}
		
		
		typename DomStack_t::template Domain<0> dom(domStack);
		
		//algorithm::Astar2Impl<DomStack_t, algorithm::Astar2SearchMode::Standard> astaralg(domStack, Json());
		Alg_t alg(domStack, jAlgConfig);

		
		bool failed = false;
		Timer timer;
		timer.start();
		
		try {	
			alg.execute(domStack.getInitState());
		
			for(auto* n = alg.mGoalNode; n; n=n->parent) {
				typename decltype(dom)::State s;
				dom.unpackState(s, n->pkd);
				
				dom.prettyPrintState(s, std::cout);
				std::cout << "\n\n";
			}
		
		} catch(std::exception const&) {
			failed = true;
		}
		timer.stop();
		
		Json jRes = alg.report();
		jRes["init_state_str"] = initStateStr;
		jRes["failed"] = failed;
		jRes["wall_time"] = timer.seconds();
		return jRes;
	}
	

	
	
	template<typename D> using AstarAlg_t_D = algorithm::Astar2Impl<D, algorithm::Astar2SearchMode::Standard>;
	template<typename D> using BugsyAlgDelay_t_D = algorithm::BugsyImpl<D, false, algorithm::BugsySearchMode::Delay>;
	template<typename D> using BugsyAlgBf_t_D = algorithm::BugsyImpl<D, false, algorithm::BugsySearchMode::BranchingFactor>;
	
	Json run_tiles_44() {

		Json jRes;
		
		std::vector<std::pair<double,double>> weights = {{1000,1}, {10000,1}, {100000,1}, {200000,1}, {300000,1}};
		
		for(unsigned i=0; i<3; i++) {
			jRes[std::to_string(i)]["astar"] = run_tiles_hr<3,3, AstarAlg_t_D>(i);
			
			/*
			for(auto& w : weights) {
				std::string algStr_delay = std::string("bugsyDelay_") +  std::to_string(w.first) + "_" + std::to_string(w.second);
				//~ std::string algStr_bf = std::string("bugsyBF_") +  std::to_string(w.first) + "_" + std::to_string(w.second);
				
				Json jAlgConfig;
				jAlgConfig["wf"] = w.first;
				jAlgConfig["wt"] = w.second;
				
				jRes[std::to_string(i)][algStr_delay] = run_tiles_hr<4,4, BugsyAlgDelay_t_D>(i, jAlgConfig);
				jRes[std::to_string(i)][algStr_delay]["utility"] = 
					w.first * jRes[std::to_string(i)][algStr_delay]["goal_g"].get<double>() +
					w.second * jRes[std::to_string(i)][algStr_delay]["wall_time"].get<double>();

				//~ jRes[std::to_string(i)][algStr_bf] = run_tiles_hr<4,4, BugsyAlgBf_t_D>(i, jAlgConfig);
				//~ jRes[std::to_string(i)][algStr_bf]["utility"] = 
					//~ w.first * jRes[std::to_string(i)][algStr_bf]["goal_g"].get<double>() +
					//~ w.second * jRes[std::to_string(i)][algStr_bf]["wall_time"].get<double>();
			}
			*/
		}
		
		return jRes;
	}
	
	
	void test_cellmap_real() {
		
		gridnav::hypernav_blocked::CellMapReal<double> cellmap(100, ",fill,10,0,1,10,0");
		cellmap.dumpHeatMap(10,10);
		
	}
}

int main(int argc, const char* argv[]) {
	//mjon661::gridnav::blocked::run();
	//mjon661::pancake::run();
	//mjon661::tiles::run();
	
	//mjon661::testgraph::run();
	
	//mjon661::run_ugsapure();
	
	//mjon661::Json res;
	//res["gridnav"] = mjon661::run_session1_gridnav();
	
	//std::cout << res.dump(4);
	
	//std::cout << mjon661::test_hypernav2().dump(4);
	//std::cout << mjon661::test_pancake().dump(4);
	//std::cout << mjon661::run_tiles_44().dump(4);
	
	//std::cout << mjon661::gridnav::cube_blocked::test_cubenav_dirs() << "\n";
	
	//std::cout << stats[0] << " " << stats[1] << " " << stats[2] << "\n";
	
	//mjon661::gridnav::hypernav_blocked::test_statepack<3>();
	mjon661::test_cellmap_real();
}
