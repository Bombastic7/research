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
		
		using AbtSearch = UGSAv3_Abt<D, L+1, Bound, StatsManager>;
		
		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using StatsAcc = typename StatsManager::template StatsAcc<L>;
		
		using BaseAbstractor = typename D::template Abstractor<L-1>;
		using BaseState = typename D::template Domain<L-1>::State;
		



		struct Node {
			Util_t ug, uf;
			Cost g;
			unsigned depth;
			PackedState pkd;
			Operator in_op, parent_op;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			Util_t uh;
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
				return a->uf == b->uf ? a->g > b->g : a->uf < b->uf;
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
		
		

		UGSAv3_Abt(D& pDomStack, UGSABehaviour<Bound>& pBehaviour, StatsManager& pStats) :
			mBehaviour			(pBehaviour),
			mStatsAcc			(pStats),
			mAbtSearch			(pDomStack, pBehaviour, pStats),
			mAbtor				(pDomStack),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain),
			mBestExactNode		(nullptr)
		{}

		
		void reset() {
			mStatsAcc.reset();
			mAbtSearch.reset();
		}
		
		void clearCache() {
			mCache.clear();
			mAbtSearch.clearCache();
		}
		
		void submitStats() {
			mStatsAcc.submit();
			mAbtSearch.submitStats();
		}
		
		/*
		void addToReport(Json& jR) {
			
			Json jAll, j;
			j["Node size"] = sizeof(Node);
			j["Wrapped Node Size"] = sizeof(typename ClosedList_t::Wrapped_t);
			j["closed fill"] = mClosedList.getFill();
			j["closed table size"] = mClosedList.size();
			j["open size"] = mOpenList.size();
			j["open capacity"] = mOpenList.capacity();
			jR[std::string("Level ") + std::to_string(L)] = j;
			
			mAbtSearch.addToReport(jR);
		}
		*/
		
		
		
		Util_t doSearch(BaseState const& pBaseState) {
			
			{
				State s0 = mAbtor(pBaseState);
				PackedState pkd0;
				
				mDomain.packState(s0, pkd0);
				
				CacheEntry* ent = mCache.retrieve(pkd0);
				
				if(ent && ent->exact) {
					mStatsAcc.s_cacheHit();
					mStatsAcc.s_end();
					return ent->uh;				
				}
				
				bool miss = mCache.get(pkd0, ent);
				
				
				if(miss) {
					ent->exact = false;
					ent->uh = mAbtSearch.doSearch(s0);
					mStatsAcc.s_cacheMiss();
					mStatsAcc.l_cacheAdd();
				}
				else
					mStatsAcc.s_cachePartial();
				
				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		Cost(0);
				n0->ug =		0;
				n0->uf = 		ent->uh;
				n0->depth =		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			Node* goalNode = nullptr;
			Util_t retUCost = 0;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retUCost = n->ug;
					goalNode = n;
					mStatsAcc.s_solutionFull();
					break;
				}
				
				if(n == mBestExactNode) {
					retUCost = n->uf;
					goalNode = n;
					mStatsAcc.s_solutionPartial();
					break;
				}
				
				expand(n, s);
			}
			
			for(auto it = mClosedList.begin(); it != mClosedList.end(); ++it) {
				Node* n = *it;
				
				if(mOpenList.contains(n))
					continue;
				
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);

				if(ent->exact)
					continue;

				Util_t upg = retUCost - n->ug;
				
				if(ent->uh < upg) {
					ent->uh = upg;
					mStatsAcc.l_cacheImprove();
				}
			}
			
			
			for(Node* n = goalNode; n != nullptr; n = n->parent) {
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				if(!ent->exact) {
					mStatsAcc.l_cacheMadeExact();
					ent->exact = true;
				}
			}
			
			mBehaviour.informPath(L, goalNode->g, goalNode->depth);
			
			mStatsAcc.s_openListSize(mOpenList.size());
			mStatsAcc.s_closedListSize(mClosedList.getFill());
			
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mBestExactNode = nullptr;


			mStatsAcc.s_end();
			return retUCost;
		}
		
		
		private:
		
		void expand(Node* n, State& s) {
			mStatsAcc.a_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mStatsAcc.a_gend();
				considerkid(n, s, ops[i]);
			}
		}
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {

			Edge		edge 	= mDomain.createEdge(pParentState, pInOp);
			
			Cost		kid_g	 	= pParentNode->g + edge.cost();
			unsigned	kid_depth 	= pParentNode->depth + 1;
			
			Util_t 		kid_ug   	= mBehaviour.compute_ug(L, kid_g, kid_depth);
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->ug > kid_ug) {
					kid_dup->uf			-= kid_dup->ug;
					kid_dup->uf			+= kid_ug;
					kid_dup->ug			= kid_ug;
					
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;
					
					kid_dup->g			= kid_g;
					kid_dup->depth		= pParentNode->depth + 1;
					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.a_reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);
					
					CacheEntry* ent = mCache.retrieve(kid_pkd);
					slow_assert(ent);
					
					if(ent->exact) {
						slow_assert(mBestExactNode);
						
						if(mBestExactNode->uf > kid_dup->uf)
							mBestExactNode = kid_dup;
					}
				}
			} else {
				
				Node* kid_node 		= mNodePool.construct();
				
				CacheEntry* ent;
				bool miss = mCache.get(kid_pkd, ent);
				
				if(miss) {
					ent->exact = false;
					ent->uh = mAbtSearch.doSearch(edge.state());
					mStatsAcc.l_cacheAdd();
				}
				
				kid_node->g 		= kid_g;
				kid_node->ug		= kid_ug;
				kid_node->depth		= kid_depth;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->uf		= kid_node->ug + ent->uh;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
				
				if(ent->exact && (!mBestExactNode || mBestExactNode->uf > kid_node->uf)) {
					mBestExactNode = kid_node;
				}
			}
			
			mDomain.destroyEdge(edge);
		}
		

		UGSABehaviour<Bound>& mBehaviour;

		StatsAcc				mStatsAcc;
		AbtSearch				mAbtSearch;
		BaseAbstractor			mAbtor;
		const Domain			mDomain;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		CacheStore_t			mCache;
		Node*					mBestExactNode;
	};
	
	
	template<typename D, unsigned Bound, typename StatsManager>
	struct UGSAv3_Abt<D, Bound, Bound, StatsManager> {
		
		UGSAv3_Abt(D& pDomStack, UGSABehaviour<Bound>& pBehaviour, StatsManager& pStats) {}
		
		Util_t doSearch(typename D::template Domain<Bound-1>::State const&) {return 0;}
		void reset() {}
		void clearCache() {}
		void submitStats() {}
	};
	
	
}}}
