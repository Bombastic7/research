#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"

#include "search/admissible_abtsearch.hpp"

namespace mjon661 { namespace algorithm {

	//Standard: plain old A*. g(n) + h(n) = f(n), where g(n) is sum of edge costs of partial path/node n, h(n) is 
	//	supplied cost heuristic. Open nodes ordered on ascending f(n), with ties broken on max g(n).
	
	//Weighted: weighted A*. g(n) + w*h(n) = f(n). Open nodes ordered as in Standard. Tie breaking as in Standard.
	//	w is user provided weight constant.
	
	//Greedy: open nodes ordered on ascending h(n). No tie breaking for equal open nodes.
	
	//Speedy: open nodes ordered on ascending d(n), where d(n) is a user supplied distance-to-go heuristic. No tie breaking.
	
	//Uninformed: open nodes ordered on ascending g(n).


	enum struct Astar2SearchMode {
		Standard, Weighted, Greedy, Speedy, Uninformed
	};
	
	enum struct Astar2HrMode {
		DomainHr, AbtHr
	};
	
	template<typename = void>
	std::string astar2SearchModeStr(Astar2SearchMode m) {
		if(m == Astar2SearchMode::Standard)
			return std::string("Standard");
		else if(m == Astar2SearchMode::Weighted)
			return std::string("Weighted");
		else if(m == Astar2SearchMode::Greedy)
			return std::string("Greedy");
		else if(m == Astar2SearchMode::Speedy)
			return std::string("Speedy");
		return std::string("Uninformed");
	}
	
	template<typename = void>
	std::string astar2HrModeStr(Astar2HrMode m) {
		if(m == Astar2HrMode::DomainHr)
			return "DomainHr";
		else if(m == Astar2HrMode::AbtHr)
			return "AbtHr";
		else
			gen_assert(false);
		return "";
	}
	



	template<typename D, Astar2SearchMode Search_Mode, Astar2HrMode Hr_Mode>
	class Astar2Impl {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		
		
		template<Astar2HrMode, typename = void>
		struct HrModule;
		
		template<typename Ign>
		struct HrModule<Astar2HrMode::DomainHr, Ign> {
			HrModule(D& pDomStack, Domain const& pDomain, Json const& jConfig) :
				mDomain(pDomain)
			{}
			
			Cost costHeuristic(State const& s) {
				return mDomain.costHeuristic(s);
			}
			
			Cost distanceHeuristic(State const& s) {
				return mDomain.distanceHeuristic(s);
			};
						
			void insertReport(Json& jReport) {}
			
			private:
			Domain const& mDomain;
		};
		
		template<typename Ign>
		struct HrModule<Astar2HrMode::AbtHr, Ign> {
			HrModule(D& pDomStack, Domain const& pDomain, Json const& jConfig) :
				mAbtSearch_cost(pDomStack, jConfig),
				mAbtSearch_dist(pDomStack, jConfig)
			{}
			
			Cost costHeuristic(State const& s) {
				return mAbtSearch_cost.getHrVal(s);
			}
			
			Cost distanceHeuristic(State const& s) {
				return mAbtSearch_dist.getHrVal(s);
			};
			
			void insertReport(Json& jReport) {
				Json j;
				j["cost_abt"] = Json();
				mAbtSearch_cost.insertReport(j.at("cost_abt"));
				j["dist_abt"] = Json();
				mAbtSearch_dist.insertReport(j.at("dist_abt"));
				jReport["hrmod"] = j;
			}
			
			private:
			AdmissibleAbtSearch<D,1,D::Top_Abstract_Level+1,true> mAbtSearch_cost;
			AdmissibleAbtSearch<D,1,D::Top_Abstract_Level+1,false> mAbtSearch_dist;
		};
		
		
		struct Node {
			Cost g, f;
			PackedState pkd;
			Node* parent;
			//unsigned expn;
			//unsigned depth;
		};
		


		struct ClosedOps {
			ClosedOps(Domain const& pDomain) :
				mDomain(pDomain)
			{}
			
			bool operator()(Node * const n, PackedState const& pkd) const {
				return mDomain.compare(n->pkd, pkd);
			}
			
			size_t operator()(Node * const n) const {
				return mDomain.hash(n->pkd);
			}
			
			size_t operator()(PackedState const& pkd) const {
				return mDomain.hash(pkd);
			}
			
			private:
			Domain const& mDomain;
		};
		
		struct OpenOps {
			bool operator()(Node * const a, Node * const b) const {
				if(Search_Mode == Astar2SearchMode::Greedy || Search_Mode == Astar2SearchMode::Speedy)
					return a->f < b->f;
				
				if(a->f != b->f)
					return a->f < b->f;
				return a->g > b->g;
			}
		};
		
		
		using OpenList_t = OpenList<Node, Node, OpenOps>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										ClosedOps,
										ClosedOps,
										Domain::Is_Perfect_Hash>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;
		
		
		
		
		
		
		Astar2Impl(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mHrModule			(pDomStack, mDomain, jConfig)
		{
			if(Search_Mode == Astar2SearchMode::Weighted)
				mHrWeight = jConfig.at("weight");
		}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;
			
			mTest_exp_f.clear();
			mTest_exp_g.clear();
			mTest_exp_h.clear();
			mTest_exp_depth.clear();
			
			mTest_lastf = 0;
		}

		
		void execute(State const& s0) {
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->parent = 	nullptr;
				//n0->expn = 		0;
				//n0->depth =		0;
				
				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			

			while(true) {
				Node* n = nullptr;
				try {
					n = mOpenList.pop();
				}
				catch(AssertException const& e) {
					throw NoSolutionException("");
				}
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					mGoalNode = n;
					std::cout << "goal h: " << mDomain.costHeuristic(s) << "\n";
					break;
				}
				
				expand(n, s);
			}
		}
		
		
		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["reopnd"] = mLog_reopnd;
			j["dups"] = mLog_dups;
			j["search_mode"] = astar2SearchModeStr(Search_Mode);
			j["hr_mode"] = astar2HrModeStr(Hr_Mode);
			
			if(Search_Mode == Astar2SearchMode::Weighted)
				j["hr_weight"] = mHrWeight;

			if(mGoalNode) {
				j["goal_g"] = mGoalNode->g;
				j["goal_f"] = mGoalNode->f;
				
				std::cout << "**";
				unsigned goal_depth = 0;
				for(Node* m = mGoalNode; m; m=m->parent) {
					goal_depth++;
					
					State s;
					mDomain.unpackState(s, m->pkd);
					std::cout << "\n" << m->g << " " << m->f << " " << mGoalNode->g - m->g << "\n";
					mDomain.prettyPrintState(s, std::cout);
					std::cout << "\n\n";
				}
				goal_depth -= 1;
				
				std::cout << "**";
				j["goal_depth"] = goal_depth;
			}

			j["exp_f_raw"] = mTest_exp_f;
			j["exp_g_raw"] = mTest_exp_g;
			j["exp_h_raw"] = mTest_exp_h;
			j["exp_depth_raw"] = mTest_exp_depth;
			
			mHrModule.insertReport(j);
			
			return j;
		}
		
		
		//private:
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;

			//~ {
				//~ mDomain.prettyPrintState(s, std::cout);
				//~ getchar();
			//~ }
			
			//mTest_exp_f.push_back(n->f);
			//mTest_exp_g.push_back(n->g);
			//mTest_exp_h.push_back(n->f - n->g);
			//mTest_exp_depth.push_back(n->depth);
			//n->expn = mLog_expd;
			
			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				Cost kid_g = n->g + edgeIt.cost();

				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->g > kid_g) {
						kid_dup->g			= kid_g;
						kid_dup->parent		= n;
						//kid_dup->expn		= -1;
						//kid_dup->depth		= n->depth + 1;
						
						evalHr(kid_dup, edgeIt.state());
						
						if(!mOpenList.contains(kid_dup)) {
							mLog_reopnd++;
						}

						mOpenList.pushOrUpdate(kid_dup);
					}
				}
				else {
					Node* kid_node 		= mNodePool.construct();

					kid_node->g 		= kid_g;
					kid_node->pkd 		= kid_pkd;
					kid_node->parent	= n;
					//kid_node->expn		= -1;
					//kid_node->depth		= n->depth + 1;
					
					evalHr(kid_node, edgeIt.state());

					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		

		void evalHr(Node* n, State const& s) {
			if(Search_Mode == Astar2SearchMode::Standard) {
				n->f = n->g + mHrModule.costHeuristic(s);
				if(!(n->f <= 39)) {
					mDomain.prettyPrintState(s, std::cout);
					std::cout << n->g << " " << n->f << "\n";
				}
				//fast_assert(n->f <= 39);//.......
			}
			else if(Search_Mode == Astar2SearchMode::Weighted)
				n->f = n->g + mHrWeight * mHrModule.costHeuristic(s);
			else if(Search_Mode == Astar2SearchMode::Greedy)
				n->f = mHrModule.costHeuristic(s);
			else if(Search_Mode == Astar2SearchMode::Speedy)
				n->f = mHrModule.distanceHeuristic(s);
			else
				n->f = n->g;
		}
		


		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		HrModule<Hr_Mode> mHrModule;
		
		Node* mGoalNode;
		double mHrWeight;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		Cost mTest_lastf;
		
		std::vector<Cost> mTest_exp_f;
		std::vector<Cost> mTest_exp_g;
		std::vector<Cost> mTest_exp_h;
		std::vector<unsigned> mTest_exp_depth;
	};
}}
