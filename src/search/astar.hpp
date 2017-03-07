#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm {


	enum AstarSearchMode {
		Standard, Weighted, Greedy, Speedy
	};
	
	template<typename = void>
	std::string astarSearchModeStr(AstarSearchMode m) {
		if(m == AstarSearchMode::Standard)
			return std::string("Standard");
		else if(m == AstarSearchMode::Weighted)
			return std::string("Weighted");
		else if(m == AstarSearchMode::Greedy)
			return std::string("Greedy");
		return std::string("Speedy");
	}


	template<typename D, AstarSearchMode Search_Mode>
	class AstarImpl {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;


		struct Node {
			Cost g, f;
			PackedState pkd;
			Operator in_op;
			Operator parent_op;
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
		
		
		
		
		
		
		AstarImpl(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			()
		{
			if(Search_Mode == AstarSearchMode::Weighted)
				mHrWeight = jConfig.at("weight");
		}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;			
		}

		
		void execute(State const& s0, Solution<D>& pSolution) {
			doSearch(s0, pSolution);
		}
		
		void doSearch(State const& s0, Solution<D>& pSolution) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
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
			j["search_mode"] = astarSearchModeStr(Search_Mode);
			
			if(Search_Mode == AstarSearchMode::Weighted)
				j["hr_weight"] = mHrWeight;

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
			mLog_expd++;
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;

				mLog_gend++;
				considerkid(n, s, ops[i]);
			}
		}
		

		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {
			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			Cost 		kid_g   = pParentNode->g + edge.cost();
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mLog_dups++;
				if(kid_dup->g > kid_g) {
					kid_dup->g			= kid_g;
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
			if(Search_Mode == AstarSearchMode::Standard)
				n->f = n->g + mDomain.costHeuristic(s);
			else if(Search_Mode == AstarSearchMode::Weighted)
				n->f = n->g + mHrWeight * mDomain.costHeuristic(s);
			else if(Search_Mode == AstarSearchMode::Greedy)
				n->f = mDomain.costHeuristic(s);
			else
				n->f = mDomain.distanceHeuristic(s);
		}
		

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		Node* mGoalNode;
		double mHrWeight;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
	};
}}
