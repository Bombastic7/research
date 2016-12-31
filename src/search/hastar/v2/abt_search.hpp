#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/hastar/v2/common.hpp"



namespace mjon661 { namespace algorithm { namespace hastarv2 {


	template<typename D, unsigned L, unsigned Bound, typename StatsManager>
	class HAstar_Abt {
		

		public:
		
		using AbtSearch = HAstar_Abt<D, L+1, Bound, StatsManager>;
		
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
			Cost g, f;
			PackedState pkd;
			Operator in_op, parent_op;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			Cost h;
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
		
		

		HAstar_Abt(D& pDomStack, StatsManager& pStats, AlgoConf<> const& pConf) :
			mStatsAcc			(pStats),
			mAbtSearch			(pDomStack, pStats, pConf),
			mAbtor				(pDomStack),
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain),
			mBestExactNode		(nullptr),
			mConf				(pConf)
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
		
		
		
		Cost doSearch(BaseState const& pBaseState) {
			
			{
				State s0 = mAbtor(pBaseState);
				PackedState pkd0;
				
				mDomain.packState(s0, pkd0);
				
				Cost fval;
				
				if(mConf.doCaching) {
					CacheEntry* ent = mCache.retrieve(pkd0);
				
					if(ent && ent->exact) {
						mStatsAcc.s_cacheHit();
						mStatsAcc.s_end();
						return ent->h;				
					}
					
					bool miss = mCache.get(pkd0, ent);
					
					if(miss) {
						ent->exact = false;
						ent->h = mAbtSearch.doSearch(s0);
						fval = ent->h;
						mStatsAcc.s_cacheMiss();
						mStatsAcc.l_cacheAdd();
					}
					else {
						fval = ent->h;
						mStatsAcc.s_cachePartial();
					}
				}
				else
					fval = mAbtSearch.doSearch(s0);				
				


				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		Cost(0);
				n0->f = 		fval;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			Node* goalNode = nullptr;
			Cost retCost = 0;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retCost = n->g;
					goalNode = n;
					mStatsAcc.s_solutionFull();
					break;
				}
				
				if(n == mBestExactNode) {
					retCost = n->f;
					goalNode = n;
					mStatsAcc.s_solutionPartial();
					break;
				}
				
				expand(n, s);
			}
			
			if(mConf.doCaching) {
				for(auto it = mClosedList.begin(); it != mClosedList.end(); ++it) {
					Node* n = *it;
					
					if(mOpenList.contains(n))
						continue;
					
					CacheEntry* ent = mCache.retrieve(n->pkd);
					slow_assert(ent);

					if(ent->exact)
						continue;

					Cost pg = retCost - n->g;
					
					if(ent->h < pg) {
						ent->h = pg;
						mStatsAcc.l_cacheImprove();
					}
				}
				
				
				for(Node* n = goalNode; n != nullptr; n = n->parent) {
					CacheEntry* ent = mCache.retrieve(n->pkd);
					slow_assert(ent);
					
					if(!ent->exact)
						mStatsAcc.l_cacheMadeExact();
					
					ent->exact = true;
				}
			}

			mStatsAcc.s_openListSize(mOpenList.size());
			mStatsAcc.s_closedListSize(mClosedList.getFill());
			
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mBestExactNode = nullptr;


			mStatsAcc.s_end();
			return retCost;
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
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->g > kid_g) {
					kid_dup->f			-= kid_dup->g;
					kid_dup->f			+= kid_g;
					kid_dup->g			= kid_g;
					
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;

					
					if(!mOpenList.contains(kid_dup)) {
						mStatsAcc.a_reopnd();
					}
					
					mOpenList.pushOrUpdate(kid_dup);
					
					if(mConf.doCaching) {
						CacheEntry* ent = mCache.retrieve(kid_pkd);
						slow_assert(ent);
						
						if(ent->exact) {
							slow_assert(mBestExactNode);
							
							if(mBestExactNode->f > kid_dup->f)
								mBestExactNode = kid_dup;
						}
					}
				}
			} else {
				
				Node* kid_node 		= mNodePool.construct();
				
				CacheEntry* ent = nullptr;
				Cost hval;
				
				if(mConf.doCaching) {
					bool miss = mCache.get(kid_pkd, ent);
				
					if(miss) {
						ent->exact = false;
						ent->h = mAbtSearch.doSearch(edge.state());
						hval = ent->h;
						mStatsAcc.l_cacheAdd();
					}
					else
						hval = ent->h;
				
				} 
				else
					hval = mAbtSearch.doSearch(edge.state());
				
				kid_node->g 		= kid_g;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->f			= kid_g + hval;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
				
				if(mConf.doCaching && ent->exact && (!mBestExactNode || mBestExactNode->f > kid_node->f)) {
					mBestExactNode = kid_node;
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
		
		AlgoConf<>				mConf;
	};
	
	
	template<typename D, unsigned Bound, typename StatsManager>
	struct HAstar_Abt<D, Bound, Bound, StatsManager> {
		
		HAstar_Abt(D& pDomStack, StatsManager& pStats, AlgoConf<> const&) {}
		
		typename D::template Domain<Bound-1>::Cost doSearch(typename D::template Domain<Bound-1>::State const&) {return 0;}
		void reset() {}
		void clearCache() {}
		void submitStats() {}
	};
	
	
}}}
