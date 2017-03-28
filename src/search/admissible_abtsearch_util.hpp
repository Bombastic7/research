#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/cache_store.hpp"

#include <iostream>



namespace mjon661 { namespace algorithm {


	//template<typename D, typename CompNodeVals_t, typename AlgInfo_t, unsigned L, unsigned Bound>
	template<typename D, unsigned L, unsigned Bound>
	class AdmissibleAbtSearch_Util {
		
		public:
		
		using AbtSearch = AdmissibleAbtSearch_Util<D, L+1, Bound>;
		
		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;

		using BaseState = typename D::template Domain<L-1>::State;

		//using Util_t = typename AlgInfo::Util_t;
		using Util_t = double;
		

		struct Node {
			Util_t g, f;
			PackedState pkd;
			Operator in_op, parent_op;
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
		
		

		AdmissibleAbtSearch_Util(D& pDomStack, Json const& jConfig) :
			mAbtSearch			(pDomStack, jConfig),
			mDomain				(pDomStack),
			mDomStack			(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain),
			mBestExactNode		(nullptr),
			mCostWeight			(1),
			mDistWeight			(0)
		{}

		
		void reset() {
			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mBestExactNode = nullptr;
		}
		
		void setWeights(Util_t wc, Util_t wd) {
			mCostWeight = wc;
			mDistWeight = wd;
			mAbtSearch.setWeights(wc, wd);
			clearCache();
		}
		
		void clearCache() {
			mCache.clear();
			mAbtSearch.clearCache();
		}


		bool doSearch_ParentState(BaseState const& pBaseState, Util_t& out_h) {
			if(mDomStack.topUsedAbstractLevel() < L) {
				out_h = 0;
				return false;
			}
			State s0 = mDomain.abstractParentState(pBaseState);
			
			return doSearch(s0, out_h);
		}
		
		bool doSearch(State const& s0, Util_t& out_h) {
			reset();
			{
				PackedState pkd0;
				
				mDomain.packState(s0, pkd0);
				
				CacheEntry* ent = mCache.retrieve(pkd0);
			
				if(ent && ent->exact) {
					out_h = ent->h;
					return false;
				}
				
				bool miss = mCache.get(pkd0, ent);
					
				if(miss) {
					ent->exact = false;
					mAbtSearch.doSearch_ParentState(s0, ent->h);
				}


				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->g = 		Cost(0);
				n0->f = 		ent->h;
				n0->in_op = 	mDomain.getNoOp();
				n0->parent_op = mDomain.getNoOp();
				n0->parent = 	nullptr;

				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			Node* goalNode = nullptr;

			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s) || n == mBestExactNode) {
					out_h = n->f;
					goalNode = n;
					
					if(mDomain.checkGoal(s))
						slow_assert(n->g == n->f);

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
				
				Util_t pg = goalNode->f - n->g;
				slow_assert(pg >= 0);
				
				if(ent->h < pg) {
					ent->h = pg;
				}
			}
			
			for(Node* n = goalNode; n != nullptr; n = n->parent) {
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				if(!ent->exact) {
					ent->exact = true;
				}
			}

			return true;
		}
		
		
		private:
		
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

			Edge		edge 		= mDomain.createEdge(pParentState, pInOp);
			Util_t		kid_g		= pParentNode->g + mCostWeight * edge.cost() + mDistWeight;

			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mLog_dups++;
				if(kid_dup->g > kid_g) {
					kid_dup->f			-= kid_dup->g;
					kid_dup->f			+= kid_g;
					kid_dup->g			= kid_g;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op	= edge.parentOp();
					kid_dup->parent		= pParentNode;

					if(!mOpenList.contains(kid_dup)) {
						mLog_reopnd++;
					}
					
					mOpenList.pushOrUpdate(kid_dup);
					
					CacheEntry* ent = mCache.retrieve(kid_pkd);
					slow_assert(ent);
						
					if(ent->exact) {
						slow_assert(mBestExactNode);
						
						if(mBestExactNode->f > kid_dup->f) {
							mBestExactNode = kid_dup;
						}
					}
				}
			} else {
				
				Node* kid_node 		= mNodePool.construct();
				
				CacheEntry* ent = nullptr;
				
				bool miss = mCache.get(kid_pkd, ent);
			
				if(miss) {
					ent->exact = false;
					mAbtSearch.doSearch_ParentState(edge.state(), ent->h);
				}

				kid_node->g 		= kid_g;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->f			= kid_g + ent->h;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
				
				if(ent->exact && (!mBestExactNode || mBestExactNode->f > kid_node->f)) {
					mBestExactNode = kid_node;
				}
			}
			
			mDomain.destroyEdge(edge);
		}


		AbtSearch				mAbtSearch;
		const Domain			mDomain;
		D const&				mDomStack;
		
		OpenList_t 				mOpenList;
		ClosedList_t 			mClosedList;
		NodePool_t 				mNodePool;
		
		CacheStore_t			mCache;
		Node*					mBestExactNode;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		Util_t mCostWeight, mDistWeight;
	};
	
	
	template<typename D, unsigned Bound>
	struct AdmissibleAbtSearch_Util<D, Bound, Bound> {
		
		using Domain = typename D::template Domain<Bound-1>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		

		AdmissibleAbtSearch_Util(D& pDomStack, Json const&) {}
		
		template<typename PV>
		bool doSearch_ParentState(State const&, PV& out_h) {
			out_h = 0;
			return true;
		}
		
		void reset() {}
		void clearCache() {}
		void setWeights(double, double) {}
	};	
}}
