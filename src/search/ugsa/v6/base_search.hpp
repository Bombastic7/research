#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <tuple>
#include <vector>
#include <map>

#include "search/openlist.hpp"
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "search/cache_store.hpp"
#include "structs/object_pool.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"

#include "search/ugsa/v6/defs.hpp"
#include "search/ugsa/v6/abt_search.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav6 {


	

	template<typename D>
	class UGSAv6_Base {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;

		using AbtSearch = UGSAv6_Abt<D, 1, 1>;
		
		struct Node {
			Cost g, f;
			unsigned depth;
			Util_t u;
			
			PackedState pkd;
			Node* parent;
			Operator in_op, parent_op;
			
			double log_expCount;
			double log_remexp;
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



		UGSAv6_Base(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mAbtSearch			(pDomStack, jConfig),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt"))
		{
			fast_assert(mParams_wf >= 0 && mParams_wt >= 0);
		}

		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mAbtSearch.reset();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mLogCurBF = 1;
			mGoalNode = nullptr;
			
		}
		
		void clearCache() {
			mAbtSearch.clearCache();
		}
		

		
		void doSearch(State const& s0, Solution<D>& pSolution) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
				n0->depth =		0;
				n0->in_op = 	mDomain.getNoOp();
				n0->parent_op = mDomain.getNoOp();
				n0->parent = 	nullptr;

				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			

			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					prepareSolution(pSolution, n);
					mGoalNode = n;
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

			j["abt"] = mAbtSearch.report();
			return j;
		}
		
		
		private:
		
		void prepareSolution(Solution<D>& sol, Node* pGoalNode) {
			std::vector<Node*> reversePath;
			
			for(Node *n = pGoalNode; n != nullptr; n = static_cast<Node*>(n->parent))
				reversePath.push_back(n);
			
			sol.states.clear();
			sol.operators.clear();
			
			for(unsigned i=reversePath.size()-1; i!=(unsigned)-1; i--) {
				State s;
				mDomain.unpackState(s, reversePath[i]->pkd);
				
				sol.states.push_back(s);
				
				if(i != reversePath.size()-1)
					sol.operators.push_back(reversePath[i]->in_op);	
				else
					fast_assert(reversePath[i]->in_op == mDomain.getNoOp());
			}
		}
		
		
		void expand(Node* n, State& s) {
			//inform expansion
			mLog_expd++;
			
			informExpansion(n);
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;

				mLog_gend++;
				considerkid(n, s, ops[i]);
			}
		}
		
		
		Util_t compRemExp(unsigned depth) {
			Util_t acc = 0;
			for(unsigned i=1; i<=depth; i++)
				acc += std::pow(mLogCurBF, depth);
			return acc;
		}
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {
			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = pParentNode->g + edge.cost();
			unsigned	kid_depth = pParentNode->depth + 1;
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mLog_dups++;
				if(kid_dup->g > kid_g) {
					kid_dup->g			= kid_g;
					kid_dup->depth		= kid_depth;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					evalHr(kid_dup, edge.state());
					
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
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				evalHr(kid_node, edge.state());

				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		void evalHr(Node* n, State const& s) {
			std::tuple<Cost, unsigned, Util_t, Util_t> abtres = 
				mAbtSearch.computeRemainingEffort_parentState(s, mLogCurBF, mOpenList.size() + 1);
			
			n->f = n->g + std::get<0>(abtres);
			n->u = mParams_wf*n->f + std::get<3>(abtres);
			
			n->log_expCount = mLog_expd;
			n->log_remexp = std::get<2>(abtres);
		}
		
		
		void informExpansion(Node* n) {
			unsigned u = std::round(n->u);
			
			if(u == mLogU1)
				mLogN1++;
			else if(u > mLogU1) {
				mLogCurBF = (double)mLogN1 / mLogN0;
				mLogN0 = mLogN1;
				mLogU0 = mLogU1;
			}
		//	else
		//		slow_assert(false);
		}
		
		
		

		
		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		AbtSearch mAbtSearch;
		Node* mGoalNode;
		Util_t mParams_wf, mParams_wt;
		
		Util_t mLogU0, mLogU1;
		unsigned mLogN0, mLogN1;
		Util_t mLogCurBF;

		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
	};



}}}
