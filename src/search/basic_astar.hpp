#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"


namespace mjon661 { namespace algorithm {

	template<typename 	DomStack,
			 bool 		Collect_Stats = true>
	
	class AstarImpl {
		public:
		
		using Domain = typename DomStack::template Domain<0>;
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
		
		struct SearchStats {
			
			void inc_expd() { if(Collect_Stats) expd++; }
			void inc_gend() { if(Collect_Stats) gend++; }
			void inc_dups() { if(Collect_Stats) dups++; }
			void inc_reopnd() { if(Collect_Stats) reopnd++; }
			
			void reset() {
				expd = gend = dups = reopnd = 0;
			}
			
			unsigned expd, gend, dups, reopnd;
		};
		
		struct CompareNodeToState {
			CompareNodeToState(Domain const& pDomain) : mDomain(pDomain) {}
			
			bool operator()(Node * const n, PackedState const& pkd) const {
				return mDomain.compare(n->pkd, pkd);
			}
			
			private:
			Domain const& mDomain;
		};
		
		struct CompareNodesCost {
			bool operator()(Node * const a, Node * const b) const {
				return a->f == b->f ? a->g > b->g : a->f < b->f;
			}
		};
		
		struct HashNode {
			HashNode(Domain const& pDomain) : mDomain(pDomain) {}
			
			size_t operator()(Node * const n) const {
				return mDomain.hash(n->pkd);
			}
			
			size_t operator()(PackedState const& pkd) const {
				return mDomain.hash(pkd);
			}
			
			private:
			Domain const& mDomain;
		};
		

		using OpenList_t = OpenList<Node, Node, CompareNodesCost>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										HashNode,
										CompareNodeToState,
										Domain::Hash_Range>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;
		
		

		AstarImpl(DomStack& pDomain) :
			mStats				(),
			mDomain				(pDomain),
			mOpenList			(CompareNodesCost()),
			mClosedList			(
									HashNode(mDomain),
									CompareNodeToState(mDomain)
								),
			mNodePool			()
		{
		}

		
		Node* doSearch(State const& s0) {			
			{
				Node* n0 = mNodePool.construct();
				
				n0->g = 		Cost(0);
				n0->f = 		mDomain.heuristicValue(s0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				mDomain.packState(s0, n0->pkd);
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			while(!mOpenList.empty()) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s))
					return n;
				
				expand(n, s);
			}
			
			return nullptr;
		}
		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mStats.reset();
		}
		
		SearchStats& stats() {
			return mStats;
		}
		
		void prepareSolution(Solution<typename DomStack::template Domain<0>>& sol, Node* pGoalNode) {
			std::vector<Node*> reversePath;
			
			for(Node *n = pGoalNode; n; n = n->parent)
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
					fast_assert(reversePath[i]->in_op == mDomain.noOp);
			}
		}
		
		Json report() {
			Json j;
			j["_all_expd"] = mStats.expd;
			j["_all_gend"] = mStats.gend;
			j["dups"] = mStats.dups;
			j["reopnd"] = mStats.reopnd;
			j["Node size"] = sizeof(Node);
			j["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["closed fill"] = mClosedList.getFill();
			j["closed table size"] = mClosedList.size();
			j["open size"] = mOpenList.size();
			j["open capacity"] = mOpenList.capacity();
			return j;
		}
		
		private:
		
		void expand(Node* n, State& s) {
			mStats.inc_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStats.inc_gend();
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
				mStats.inc_dups();
				if(kid_dup->g > kid_g) {
					kid_dup->f 		   -= kid_dup->g;
					kid_dup->f 		   += kid_g;
					kid_dup->g 		   = kid_g;
					kid_dup->in_op	   = pInOp;
					kid_dup->parent_op = edge.parentOp();
					kid_dup->parent	   = pParentNode;
					
					if(!mOpenList.contains(kid_dup))
						mStats.inc_reopnd();
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->f 		= kid_g + mDomain.heuristicValue(edge.state());
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}

		SearchStats 		mStats;
		Domain				mDomain;
		
		OpenList_t 			mOpenList;
		ClosedList_t 		mClosedList;
		NodePool_t 			mNodePool;
	};
	
}}
