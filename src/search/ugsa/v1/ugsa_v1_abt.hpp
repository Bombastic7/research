#pragma once

#include <string>
#include "search/ugsa/v1/common.hpp"
#include "search/ugsa/v1/cache.hpp"
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav1 {



	template<	typename D,
				bool Collect_Stats>
	
	class UGSAv1_abt {
		public:
		
		using Domain = typename D::template Domain<1>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using CacheStore = CacheStore_basic<Domain>;
		using CacheEntry = typename CacheStore::CacheEntry;
		
		
		struct Node {
			Util_t ug, uf;
			unsigned depth;
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
				return a->uf == b->uf ? a->ug > b->ug : a->uf < b->uf;
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
		
		

		UGSAv1_abt(D& pDomStack, UGSAConfig const& pConfig, UGSABehaviour const& pBehaviour) :
			mConfig				(pConfig),
			mBehaviour			(pBehaviour),
			mStats				(),
			mDomain				(pDomStack),
			mOptNode			(nullptr),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain),
			mTotExpd			(0)
		{}


		Util_t getUtil(State const& s0, Util_t initUg) {
			
			PackedState pkd;
			mDomain.packState(s0, pkd);
			CacheEntry* ent = mCache.get(pkd);
			
			if(ent && ent->exact) {
				return initUg + ent->uh;
			}
			
			Node* n = doSearch(s0, initUg);
			Util_t retUtil = n->uf;
			mTotExpd += mStats.expd;
			reset();
			return retUtil;
		}
		
		void clearCache() {
			mCache.clear();
		}
		
		unsigned totExpd() {
			return mTotExpd;
		}
		
		private:
		
		Node* doSearch(State const& s0, Util_t initUg) {			
			{
			
				
				Node* n0 = mNodePool.construct();
				
				n0->ug = 		initUg;
				n0->depth =		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				mDomain.packState(s0, n0->pkd);
				
				CacheEntry* ent;
				mCache.find(n0->pkd, ent);
				n0->uf = n0->ug + ent->uh;
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			Node* retNode = nullptr;
			while(true) {				
				Node* n = mOpenList.pop();
				
				if(n == mOptNode) {
					retNode = n;
					break;
				}
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retNode = n;
					break;
				}
				
				expand(n, s);
			}
			
			cacheSearchResults(retNode);
			
			return retNode;
		}
		
		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mStats.reset();
			mOptNode = nullptr;
		}
		
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

			Edge		edge		= mDomain.createEdge(pParentState, pInOp);
			unsigned	kid_depth 	= pParentNode->depth + 1;
			
			Util_t		kid_ug  	= 
									pParentNode->ug + mConfig.wf * edge.cost()
									+
									mConfig.wt * 
										mBehaviour.getExpTime() * 
										mBehaviour.remainingExpansions(
											mBehaviour.abtDist(kid_depth)
										);

			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStats.inc_dups();
				if(kid_dup->ug > kid_ug) {
					kid_dup->ug			= kid_ug;
					kid_dup->depth		= kid_depth;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op 	= edge.parentOp();
					kid_dup->parent	   	= pParentNode;
					
					kid_dup->uf		   	-= kid_dup->ug;
					kid_dup->uf		   	+= kid_ug;
					
					if(!mOpenList.contains(kid_dup))
						mStats.inc_reopnd();
					
					CacheEntry* ent = mCache.get(kid_pkd);
					slow_assert(ent);
					
					if(ent->exact) {
						slow_assert(mOptNode);
						if(kid_dup->uf < mOptNode->uf)
							mOptNode = kid_dup;
					}
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				Node* kid_node 		= mNodePool.construct();

				CacheEntry* ent;
				mCache.find(kid_pkd, ent);

				kid_node->ug 		= kid_ug;
				kid_node->uf 		= kid_ug + ent->uh;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				if(ent->exact) {
					if(!mOptNode || kid_node->uf < mOptNode->uf)
						mOptNode = kid_node;
				}
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		
		void cacheSearchResults(Node* goalNode) {
			using It_t = typename ClosedList_t::iterator;
			
			for(It_t it = mClosedList.begin(); it != mClosedList.end(); ++it) {
				Node* n = *it;
				CacheEntry* ent = mCache.get(n->pkd);
				glacial_assert(ent);
				
				if(ent->exact)
					continue;
				
				Cost pg = goalNode->uf - n->ug;
				
				if(ent->uh < pg) {
					ent->uh = pg;
				}
			}
			
			for(Node* n = goalNode; n; n = n->parent) {
				CacheEntry* ent = mCache.get(n->pkd);
				glacial_assert(ent);
					
				ent->exact = true;
			}
		}
		
		UGSAConfig			mConfig;
		UGSABehaviour		mBehaviour;

		SearchStats 		mStats;
		const Domain		mDomain;
		Node*				mOptNode;
		
		OpenList_t 			mOpenList;
		ClosedList_t 		mClosedList;
		NodePool_t 			mNodePool;
		CacheStore			mCache;
		
		unsigned mTotExpd;
	};
	
}}}
