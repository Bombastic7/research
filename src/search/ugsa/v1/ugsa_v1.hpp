#pragma once

#include <string>
#include "search/ugsa/v1/common.hpp"
#include "search/ugsa/v1/ugsa_v1_abt.hpp"
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav1 {



	template<	typename D,
				bool Collect_Stats>
	class UGSAv1_impl {
		public:
		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using Abstractor = typename D::template Abstractor<0>;
		using AbtState = typename D::template Domain<1>::State;
		
		
		struct Node {
			Cost g;
			Util_t u;
			PackedState pkd;
			Operator in_op;
			Operator parent_op;
			Node* parent;
			unsigned expdAtGen;
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
		
		struct ClosedOps {
			ClosedOps(Domain const& pMethods) :
				mDomain(pMethods)
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
				return a->u == b->u ? a->g > b->g : a->u < b->u;
			}
		};
		
		
		

		using OpenList_t = OpenList<Node, Node, OpenOps>;
		
		using ClosedList_t = ClosedList<Node, 
										typename OpenList_t::Wrapped_t, 
										PackedState, 
										ClosedOps,
										ClosedOps,
										Domain::Hash_Range>;
									  
		using NodePool_t = NodePool<Node, typename ClosedList_t::Wrapped_t>;
		
		

		UGSAv1_impl(D& pDomStack, Json const& j) :
			mConfig				(j),
			mBehaviour			(),
			mAbtSearch			(pDomStack, mConfig, mBehaviour),
			mStats				(),
			mDomain				(pDomStack),
			mAbtor				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			()
		{
		}

		
		Node* doSearch(State const& s0) {
			mBehaviour.reset();
				
			{
				Node* n0 = mNodePool.construct();
				
				n0->g = 		Cost(0);
				n0->u = 		computeUtil(s0, n0->g);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				n0->expdAtGen = 0;
				
				mDomain.packState(s0, n0->pkd);
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			while(true) {
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s))
					return n;
				
				expand(n, s);
				
				if(mBehaviour.shouldUpdate(mStats.expd)) {
					mBehaviour.update(mStats.expd);
					mAbtSearch.clearCache();
					resortOpenList();
				}
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
		
		void prepareSolution(Solution<typename D::template Domain<0>>& sol, Node* pGoalNode) {
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
			j["expd"] = mStats.expd + mAbtSearch.totExpd();
			j["base expd"] = mStats.expd;
			j["gend"] = mStats.gend;
			j["dups"] = mStats.dups;
			j["reopnd"] = mStats.reopnd;
			j["Node size"] = sizeof(Node);
			j["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["closed fill"] = mClosedList.getFill();
			j["closed table size"] = mClosedList.size();
			j["open size"] = mOpenList.size();
			j["open capacity"] = mOpenList.capacity();
			j["behaviour"] = mBehaviour.report();
			return j;
		}
		
		private:
		
		void expand(Node* n, State& s) {
			mStats.inc_expd();
			mBehaviour.putExpDelay(mStats.expd - n->expdAtGen);
			
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
					kid_dup->g 		   = kid_g;
					kid_dup->in_op	   = pInOp;
					kid_dup->parent_op = edge.parentOp();
					kid_dup->parent	   = pParentNode;
					kid_dup->u		   = computeUtil(edge.state(), kid_g);
					
					if(!mOpenList.contains(kid_dup)) {
						kid_dup->expdAtGen = mStats.expd;
						mStats.inc_reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				Node* kid_node 		= mNodePool.construct();

				kid_node->g 		= kid_g;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->expdAtGen = mStats.expd;
				kid_node->u			= computeUtil(edge.state(), kid_g);
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		Util_t computeUtil(State const& pState, Cost pG) {
			AbtState abtState = mAbtor(pState);
			double ug = pG * mConfig.wf;
			
			return mAbtSearch.getUtil(abtState, ug);
			//return pG;
		}
		
		void resortOpenList() {
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				
				State s;
				mDomain.unpackState(s, n->pkd);
				
				n->u = computeUtil(s, n->g);
			}
			mOpenList.reinit();
		}
		
		
		UGSAConfig						mConfig;
		UGSABehaviour					mBehaviour;
		UGSAv1_abt<D, Collect_Stats>	mAbtSearch;

		SearchStats 					mStats;
		const Domain					mDomain;
		Abstractor						mAbtor;
		
		OpenList_t 						mOpenList;
		ClosedList_t 					mClosedList;
		NodePool_t 						mNodePool;
	};
	
	
	template<typename D>
	struct UGSAv1  {
		
		using Domain = typename D::template Domain<0>;
		
		UGSAv1(D& pDomStack, Json const& j) :
			mAlgo(pDomStack, j),
			mDomain(pDomStack)
		{}
		
		void execute(Solution<Domain>& pSol) {
			auto s0 = mDomain.createState();
			auto goalNode = mAlgo.doSearch(s0);
			
			mAlgo.prepareSolution(pSol, goalNode);
		}
		
		void reset() {
			mAlgo.reset();
		}
		
		Json report() {
			return mAlgo.report();
		}
		
		private:
		UGSAv1_impl<D, true> mAlgo;
		Domain mDomain;
	};
}}}
