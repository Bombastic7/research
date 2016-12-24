#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/ugsa/v3/common.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav3 {


	template<typename D, unsigned L, unsigned Bound, typename StatsManager>
	class UGSAv3_Abt {
		

		public:
		
		//using AbtSearch = UGSAv3_Abt<D, 1, Top+1, StatsManager>
		
		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<L>;
		
		using BaseAbstractor = typename D::template Abstractor<L-1>;
		
		

		struct Node {
			Util_t g, f;
			PackedState pkd;
			Operator in_op; parent_op;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			Util_t h;
			bool exact;
		};
		
		using CacheStore_t = CacheStore<Domain, CacheEntry>;
		
		
		
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
				return a->f == b->f ? a->g > b->g : a->f < b->f;
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
		
		

		UGSAv3_Abt(D& pDomStack, UGSABehaviour& pBehaviour, StatsManager& pStats) :
			mBehaviour			(pBehaviour),
			mStatsAcc			(pStats),
			mAbtSearch			(pDomStack, mBehaviour, pStats),
			mAbtor				(pDomStack),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain)
		{}

		
		void reset() {
			mOpenList.clear();
		}
		
		void addToReport(Json& jR) {
			Json jAll, j;
			j["reopnd"] = mStats.reopnd;
			j["Node size"] = sizeof(Node);
			j["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["closed fill"] = mClosedList.getFill();
			j["closed table size"] = mClosedList.size();
			j["open size"] = mOpenList.size();
			j["open capacity"] = mOpenList.capacity();
			j["config"] = mConfig.report();
			j["behaviour"] = mBehaviour.report();
			jR[std::string("Level ") + std::to_string(L)] = j;
			mAbtSearch.addToReport(jR);
		}

		
		
		
		Util_t doSearch(BaseState const& pBaseState) {
			
			{
				State s0 = mAbtor(pBaseState);
				PackedState pkd0;
				
				mDomain.packState(s0, pkd0);
				
				CacheEntry* ent = mCacheStore.retrieve(pkd);
				
				if(ent && ent->exact) {
					return ent->h;
				}
				
				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		Cost(0);
				n0->f = 		computeHeuristic(s0);
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			Cost retCost;
			Node* goalNode;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retCost = n->g;
					mStatsAcc.submit();
					break;
				}
				
				expand(n, s);
				/*
				if(mBehaviour.shouldUpdate(mStats.expd)) {
					mBehaviour.update(mStats.expd);
					mAbtSearch.clearCacheRec();
					resortOpenList();
				}*/
			}
			
			for(auto it = mClosedList.begin(); it != mClosedList.end(); ++it) {
				Node* n = *it;
				
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				if(ent->exact)
					continue;
				
				Util_t pg = goalNode->f - n->g;
				
				if(ent->h < pg)
					ent->h = pg;
			}
			
			
			for(Node* n = goalNode; n != nullptr; n = n.parent) {
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				ent->exact = true;
			}
			
			mStatsAcc.submit();
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
		}
		
		
		private:
		
		void expand(Node* n, State& s) {
			mStatsAcc.expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStatsAcc.gend();
				considerkid(n, s, ops[i]);
			}
		}
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {

			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			Util_t 		kid_g   = mBehaviour.computeAbtG(L, pParentNode->g, edge.cost());
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.dups();
				if(kid_dup->g > kid_g) {
					
					kid_dup->f			-= kid_dup->g;
					kid_dup->f			+= kid_g;
					kid_dup->g			= kid_g;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				
				CacheEntry* ent;
				bool miss = mCache.get(kid_pkd, ent)
				
				if(miss) {
					ent->exact = false;
					ent->h = mAbtSearch.doSearch(edge.state());
				}
				
				
				Node* kid_node 		= mNodePool.construct();
				
				kid_node->g 		= kid_g;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->expdAtGen = mStats.expd;
				kid_node->f			= kid->g + ent->h;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		

		
		/*
		void resortOpenList() {
			for(unsigned i=0; i<mOpenList.size(); i++) {
				Node* n = mOpenList.at(i);
				
				State s;
				mDomain.unpackState(s, n->pkd);
				
				n->u = computeUtil(s, n->g);
			}
			mOpenList.reinit();
		}
		*/
		
		

		UGSABehaviour			mBehaviour;
		StatsAcc				mStatsAcc;
		AbtSearch				mAbtSearch;
		BaseAbstractor			mAbtor;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		CacheStore_t			mCache;
	};
	
	
	template<typename D, unsigned Bound, typename StatsManager>
	struct UGSAv3_Abt<D, Bound, Bound, StatsManager> {
		
		UGSAv3_Abt(D& pDomStack, UGSABehaviour& pBehaviour, StatsManager& pStats) {}
		
		Util_t doSearch() {return 0;}
	};
	
	
}}}
