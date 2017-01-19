#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/ugsa/v5/common.hpp"
#include "search/ugsa/v5/abt2_search.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav5 {


	template<typename D, unsigned L, unsigned Bound, typename StatsManager>
	class UGSAv5_Abt {
		static_assert(L == 1, "");

		public:
		
		using AbtSearch = UGSAv5_Abt2<D, L+1, Bound, StatsManager>;
		
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

		static const unsigned Null_Dist = (unsigned)-1;
		

		struct Node {
			Cost g, f;
			unsigned depth;
			PackedState pkd;
			Operator in_op, parent_op;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			Cost h;
			unsigned d;
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
				if(a->f != b->f)
					return a->f < b->f;
				
				else if(a->g != b->g)
					return a->g > b->g;

				return a->depth < b->depth;
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
		
		

		UGSAv5_Abt(D& pDomStack, Json const& jConfig, StatsManager& pStats) :
			mStatsAcc			(pStats),
			mAbtSearch			(pDomStack, jConfig, pStats),
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
			mAbtSearch.submitStats();
		}
		
		
		
		bool doSearch(BaseState const& pBaseState, Cost& out_h, unsigned& out_d) {
			{
				State s0 = mAbtor(pBaseState);
				PackedState pkd0;
				
				mDomain.packState(s0, pkd0);
				
				CacheEntry* ent = mCache.retrieve(pkd0);
			
				if(ent && ent->exact) {
					mStatsAcc.s_cacheHit();
					mStatsAcc.s_end();
					out_h = ent->h;
					out_d = ent->d;
					slow_assert(out_d != Null_Dist);
					return false;
				}
				
				bool miss = mCache.get(pkd0, ent);
					
				if(miss) {
					ent->exact = false;
					ent->d = Null_Dist;
					mAbtSearch.doSearch(s0, ent->h);
					mStatsAcc.s_cacheMiss();
					mStatsAcc.l_cacheAdd();
				}
				else {
					mStatsAcc.s_cachePartial();
				}


				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		Cost(0);
				n0->f = 		ent->h;
				n0->depth =		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			Node* goalNode = nullptr;
			Cost retCost;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					out_h = n->g;
					out_d = n->depth;
					retCost = n->g;
					goalNode = n;
					mStatsAcc.s_solutionFull();
					break;
				}
				
				if(n == mBestExactNode) {
					out_h = n->f;
					out_d = n->depth;
					retCost = n->f;
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

				slow_assert(ent->d == Null_Dist);
				
				Cost pg = retCost - n->g;
				
				if(ent->h < pg) {
					ent->h = pg;
					mStatsAcc.l_cacheImprove();
				}
			}
			
			for(Node* n = goalNode; n != nullptr; n = n->parent) {
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				unsigned dist = goalNode->depth - n->depth;
				
				if(!ent->exact) {
					mStatsAcc.l_cacheMadeExact();
					slow_assert(ent->d == Null_Dist);
					ent->d = dist;
					ent->exact = true;
				}
			}

			mStatsAcc.s_openListSize(mOpenList.size());
			mStatsAcc.s_closedListSize(mClosedList.getFill());
			
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mBestExactNode = nullptr;

			slow_assert(out_d != Null_Dist);
			
			mStatsAcc.s_end();
			return true;
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

			Edge		edge 		= mDomain.createEdge(pParentState, pInOp);
			Cost		kid_g		= pParentNode->g + edge.cost();
			unsigned	kid_depth 	= pParentNode->depth + 1;

			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->g > kid_g) {
					kid_dup->f			-= kid_dup->g;
					kid_dup->f			+= kid_g;
					kid_dup->g			= kid_g;
					kid_dup->depth		= kid_depth;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;

					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.a_reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);
					
					CacheEntry* ent = mCache.retrieve(kid_pkd);
					slow_assert(ent);
						
					if(ent->exact) {
						slow_assert(mBestExactNode);
						
						if(mBestExactNode->f > kid_dup->f) {
							mBestExactNode = kid_dup;
							slow_assert(ent->d != Null_Dist);
							kid_dup->depth += ent->d;
						}
					}
				}
			} else {
				
				Node* kid_node 		= mNodePool.construct();
				
				CacheEntry* ent = nullptr;
				
				bool miss = mCache.get(kid_pkd, ent);
			
				if(miss) {
					ent->exact = false;
					ent->d = Null_Dist;
					mAbtSearch.doSearch(edge.state(), ent->h);
					mStatsAcc.l_cacheAdd();
				}

				kid_node->g 		= kid_g;
				kid_node->depth		= kid_depth;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->f			= kid_g + ent->h;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
				
				if(ent->exact && (!mBestExactNode || mBestExactNode->f > kid_node->f)) {
					mBestExactNode = kid_node;
					slow_assert(ent->d != Null_Dist);
					kid_node->depth += ent->d;
				}
			}
			
			mDomain.destroyEdge(edge);
		}
		


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
	struct UGSAv5_Abt<D, Bound, Bound, StatsManager> {
		
		using Domain = typename D::template Domain<Bound-1>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		
		UGSAv5_Abt(D& pDomStack, Json const&, StatsManager& pStats) {}
		
		bool doSearch(State const&, Cost& out_h, unsigned& out_d) {
			out_h = 0;
			out_d = 0;
			return true;
		}
		
		void reset() {}
		void clearCache() {}
		void submitStats() {}
	};	
}}}
