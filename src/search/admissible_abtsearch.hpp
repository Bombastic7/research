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


	template<typename D, unsigned L, unsigned Bound, bool Min_Cost>
	class AdmissibleAbtSearch {

		static const bool Do_Print_Expansions = false;//true;
		
		public:
		
		using AbtSearch = AdmissibleAbtSearch<D, L+1, Bound, Min_Cost>;
		
		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;

		using BaseState = typename D::template Domain<L-1>::State;


		template<bool B, typename = void>
		struct NodeImpl {
			using PrimVal_t = Cost;
			
			Cost g, f;
			PackedState pkd;
			Operator in_op, parent_op;
			NodeImpl<B>* parent;
			
			Cost& x() {return g;}
			Cost& y() {return f;}
		};
		
		template<typename Ign>
		struct NodeImpl<false, Ign> {
			using PrimVal_t = unsigned;
			
			unsigned depth, dtot;
			PackedState pkd;
			Operator in_op, parent_op;
			NodeImpl<false, Ign>* parent;
			
			unsigned& x() {return depth;}
			unsigned& y() {return dtot;}
		};

		using Node = NodeImpl<Min_Cost>;
		using PrimVal_t = typename Node::PrimVal_t;
		

		struct CacheEntry {
			PackedState pkd;
			PrimVal_t h;
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
				if(a->y() != b->y())
					return a->y() < b->y();
				return a->x() > b->x();
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
		
		

		AdmissibleAbtSearch(D& pDomStack, Json const& jConfig) :
			mAbtSearch			(pDomStack, jConfig),
			mDomain				(pDomStack),
			mDomStack			(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain),
			mBestExactNode		(nullptr)
		{}

		
		void reset() {
			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mBestExactNode = nullptr;
		}
		
		void clearCache() {
			mCache.clear();
			mAbtSearch.clearCache();
		}


		
		bool doSearch(BaseState const& pBaseState, PrimVal_t& out_h) {
			reset();
			{
				State s0 = mDomain.abstractParentState(pBaseState);
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
					mAbtSearch.doSearch(s0, ent->h);
				}


				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->x() = 		Cost(0);
				n0->y() = 		ent->h;
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
					out_h = n->y();
					goalNode = n;
					
					if(mDomain.checkGoal(s))
						slow_assert(n->y() == n->x());

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
				
				PrimVal_t pg = goalNode->y() - n->x();
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
			PrimVal_t	kid_x		= Min_Cost ? pParentNode->x() + edge.cost() : pParentNode->x() + 1;

			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mLog_dups++;
				if(kid_dup->x() > kid_x) {
					kid_dup->y()		-= kid_dup->x();
					kid_dup->y()		+= kid_x;
					kid_dup->x()		= kid_x;
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
						
						if(mBestExactNode->y() > kid_dup->y()) {
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
					mAbtSearch.doSearch(edge.state(), ent->h);
				}

				kid_node->x() 		= kid_x;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->y()		= kid_x + ent->h;
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
				
				if(ent->exact && (!mBestExactNode || mBestExactNode->y() > kid_node->y())) {
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
	};
	
	
	template<typename D, unsigned Bound, bool Min_Cost>
	struct AdmissibleAbtSearch<D, Bound, Bound, Min_Cost> {
		
		using Domain = typename D::template Domain<Bound-1>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		

		AdmissibleAbtSearch(D& pDomStack, Json const&) {}
		
		template<typename PV>
		bool doSearch(State const&, PV& out_h) {
			out_h = 0;
			return true;
		}
		
		void reset() {}
		void clearCache() {}
	};	
}}
