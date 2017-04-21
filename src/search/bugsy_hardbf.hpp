#pragma once

#include <map>
#include <string>
#include <cstdio>
#include <queue>

#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/time.hpp"



/*

OP_distVal = { domhr, abthr, depth+domhr, depth+abthr }


BugsyHardBF(s0):
	n0 = {state=s0, g=0, depth=0, u=0, parent=null}
	
	openlist.push(n0)
	closedlist.push(n0)
	
	loop:
		n = openlist.pop()
		//return if n.state is goal
		
		foreach edge (c,cost) of n.state:
			//if c is state of parent of n, continue
			
			nc = {state=c, g=n+cost, depth=n+1, u=eval_u(...), parent=n}
			
			if(c has already been encountered and nc.u is lower than duplicate's):
				replace duplicate with nc, push/update nc onto openlist


eval_u(...):
	remexp = bf ** abthr(s, bf, rollingdelay)
	wf * g + wt * exptime * remexp



d = k * delay * D

 
*/

namespace mjon661 { namespace algorithm {




	template<typename D>
	class BugsyHardBF {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		
		struct Node {
			Cost g, f;
			unsigned depth;
			double u;
			unsigned expdAtGen;
			PackedState pkd;
			Node* parent;
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
				if(a->u != b->u)
					return a->u < b->u;
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
		
		
		
		
		
		
		BugsyHardBF(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mParam_wf			(jConfig.at("wf")),
			mParam_wt			(jConfig.at("wt")),
			mParam_bf			(jConfig.at("bf")),
			mParam_stExpTime	(jConfig.at("exptime").get<double>())
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;		

			mLog_curExpTime = mParam_stExpTime;

			mLog_searchTimer.start();

			mLog_pastExpTimes.clear();
			mLog_pastExpTimes.push_back(mLog_curExpTime);
		}
		
		void execute(State const& s0) {
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->depth =		0;
				n0->parent = 	nullptr;
				
				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			while(true) {
				if(mLog_expd % 10000 == 0 && debugCheckMemLimit())
					throw NoSolutionException("memlimit");
				
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
					break;
				}
				
				expand(n, s);
			}
			mLog_searchTimer.stop();
		}
		
		
		
		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["reopnd"] = mLog_reopnd;
			j["dups"] = mLog_dups;
			j["wf"] = mParam_wf;
			j["wt"] = mParam_wt;
			j["hard_bf"] = mParam_bf;
			
			j["wall_time"] = mLog_searchTimer.wallSeconds();
			j["cpu_time"] = mLog_searchTimer.cpuSeconds();
			
			fast_assert(mGoalNode);
			
			j["goal_g"] = mGoalNode->g;
			j["goal_f"] = mGoalNode->f;
			
			unsigned goal_depth = 0;
			for(Node* m = mGoalNode->parent; m; m=m->parent) {
				goal_depth++;
			}
			
			j["goal_depth"] = goal_depth;

			j["past_delays"] = mLog_pastDelays;
			j["past_exptimes"] = mLog_pastExpTimes;
			return j;
		}
		
		
		//private:
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			
			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				Cost kid_g = n->g + edgeIt.cost();
				unsigned kid_depth = n->depth + 1;
				
				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->g > kid_g) {
						kid_dup->g			= kid_g;
						kid_dup->depth		= kid_depth;
						kid_dup->parent		= n;
						kid_dup->expdAtGen	= mLog_expd;
						
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
					kid_node->depth		= kid_depth;
					kid_node->pkd 		= kid_pkd;
					kid_node->parent	= n;
					kid_node->expdAtGen	= mLog_expd;
					
					evalHr(kid_node, edgeIt.state());

					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		

		void evalHr(Node* n, State const& s) {
			n->f = n->g + mDomain.costHeuristic(s);			
			n->u = mParam_wf * n->f + (unsigned) std::round(mParam_wt * mLog_curExpTime * std::pow(mParam_bf, n->depth));
		}
		
	
		
		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		const double mParam_wf, mParam_wt, mParam_bf, mParam_stExpTime;
		
		Node* mGoalNode;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		double mLog_curExpTime;
		
		std::vector<double> mLog_pastDelays, mLog_pastExpTimes;
		
		Timer mLog_resortTimer, mLog_searchTimer;
	};
}}
