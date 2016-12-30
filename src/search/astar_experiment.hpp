#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"


namespace mjon661 { namespace algorithm {

	template<typename 	DomStack>
	
	class AstarExpImpl {
		public:
		
		using Domain = typename DomStack::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;

		static const bool Collect_Stats = true;
		
		struct Node {
			Cost g, f;
			PackedState pkd;
			Operator in_op;
			Operator parent_op;
			Node* parent;
			unsigned depth;
		};
		
		struct SearchStats {
			
			struct SamplePair {
				unsigned depth;
				Cost f;
			};
			
			void inc_expd() { if(Collect_Stats) expd++; }
			void inc_gend() { if(Collect_Stats) gend++; }
			void inc_dups() { if(Collect_Stats) dups++; }
			void inc_reopnd() { if(Collect_Stats) reopnd++; }
			
			void put_bf_sample(unsigned depth, Cost f) {
				mBFsamples.push_back(SamplePair{.depth=depth, .f=f});
			}
			
			void dump(std::ostream& out) {
				for(unsigned i=0; i<mBFsamples.size(); i++) {
					out << mBFsamples[i].depth << " " << mBFsamples[i].f << "\n";
				}
			}

			void reset() {
				expd = gend = dups = reopnd = 0;
				mBFsamples.clear();
			}
			
			unsigned expd, gend, dups, reopnd;
			std::vector<SamplePair> mBFsamples;
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
		
		

		AstarExpImpl(DomStack& pDomain) :
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
				n0->depth = 0;
				
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
			j["expd"] = mStats.expd;
			j["gend"] = mStats.gend;
			j["dups"] = mStats.dups;
			j["reopnd"] = mStats.reopnd;
			j["Node size"] = sizeof(Node);
			j["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["closed fill"] = mClosedList.getFill();
			j["closed table size"] = mClosedList.size();
			j["open size"] = mOpenList.size();
			j["open capacity"] = mOpenList.capacity();
			
			std::ofstream ofs("astar_exp.txt");
			
			if(ofs) {
				mStats.dump(ofs);
			}
			return j;
		}
		
		private:
		
		void expand(Node* n, State& s) {
			mStats.inc_expd();
			
			mStats.put_bf_sample(n->depth, n->f);
			
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
					kid_dup->depth = pParentNode->depth + 1;
					
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
				kid_node->depth = pParentNode->depth + 1;
				
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


	template<typename DomStack>
	class AstarExperiment {
		public:
		
		using Astar_t =			AstarExpImpl<DomStack>;
		using Domain =			typename DomStack::template Domain<0>;
		using State = 			typename Domain::State;
		using Node =			typename Astar_t::Node;
			
		AstarExperiment(DomStack& pDomStack, Json const&) :
			mDomain(pDomStack),
			mAlgo(pDomStack)
		{}
		
		void execute(Solution<typename DomStack::template Domain<0>>& pSolution) {
			State s0 = mDomain.createState();
			
			Node* goalNode = mAlgo.doSearch(s0);
			
			mAlgo.prepareSolution(pSolution, goalNode);
		}
		
		void reset() {
			mAlgo.reset();
		}
		
		Json report() {
			return mAlgo.report();
		}
		
		Domain mDomain;
		Astar_t mAlgo;
	};	
	
}}



