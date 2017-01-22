#pragma once

#include <limits>
#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"

#include "search/ugsa/v5/common.hpp"
#include "search/ugsa/v5/cache.hpp"
#include "search/ugsa/v5/abt2_search.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav5 {


	/*
	 * 	Informed search.
	 * 	If Minimise_Cost is true, doSearch() finds the minimum cost path. If false, doSearch finds the minimum length path.
	 * 	doSearch() returns the path cost and path length for the found path.
	 * 	Homomorphic abstraction assumed.
	 * 
	 * 	Each Node tracks cost-so-far (aliased as x()), cost-so-far + heuristic (y()), and the other value as w().
	 * 
	 * 	Minimise_Cost == true:
	 * 		x() = g
	 * 		y() = g + h
	 * 		w() = depth
	 * 		getprim() returns g
	 * 		getsec() returns depth
	 * 
	 * 	Minimise_Cost == false:
	 * 		x() = depth
	 * 		y() = depth + d,	where d is a distance-to-go heuristic value.
	 * 		w() = g
	 * 		getprim() returns depth
	 * 		getsec() returns g
	 * 
	 * 	CacheEntry keeps cost and distance values (h and d members) per state. 
	 * 	If Minimise_Cost is true, prim() is h, sec() is d. Otherwise its vice-versa.
	 * 	h and d may be lower bounds or exact.
	 * 	
	 * 	Algorithm uses Optimal path caching (path from init to goal gives exact cost-to-go for states on path),	
	 * 	and P-g heuristic for nodes expanded that aren't on the open list when goal was found 
	 * 	([goal node cost] - [n cost] = lower bound for [cost-to-go of n]).
	 * 
	 * 
	 */



	template<typename D, unsigned L, unsigned Bound, bool Minimise_Cost, typename StatsManager>
	class UGSAv5_Abt {

		public:
		
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


		template<bool B, typename = void>
		struct NodeImpl {
			using AbtSearch = UGSAv5_Abt2<D, L+1, Bound, true, StatsManager>;
			using PrimVal_t = Cost;
			using SecVal_t = unsigned;
			
			Cost g, f;
			unsigned depth;
			
			static Cost& getprim(Cost& pg, unsigned& pdepth) {return pg;}
			static unsigned& getsec(Cost& pg, unsigned& pdepth) {return pdepth;}
			Cost& y() {return f;}
			Cost& x() {return g;}
			unsigned& w() {return depth;}
			
			PackedState pkd;
			Operator in_op, parent_op;
			NodeImpl<B>* parent;
		};
		
		template<typename Ign>
		struct NodeImpl<false, Ign> {
			using AbtSearch = UGSAv5_Abt2<D, L+1, Bound, false, StatsManager>;
			using PrimVal_t = unsigned;
			using SecVal_t = Cost;
			
			Cost g;
			unsigned depth, dtot;
			
			static unsigned& getprim(Cost& pg, unsigned& pdepth) {return pdepth;}
			static Cost& getsec(Cost& pg, unsigned& pdepth) {return pg;}
			unsigned& y() {return dtot;}
			unsigned& x() {return depth;}
			Cost& w() {return g;}
			
			PackedState pkd;
			Operator in_op, parent_op;
			NodeImpl<false, Ign>* parent;
		};
		
		using Node = NodeImpl<Minimise_Cost>;
		using AbtSearch = typename Node::AbtSearch;
		using PrimVal_t = typename Node::PrimVal_t;
		using SecVal_t = typename Node::SecVal_t;
		
		static const SecVal_t Null_Sec = std::numeric_limits<SecVal_t>::max();
		
		
		template<bool B, typename = void>
		struct CacheEntryImpl {
			PackedState pkd;
			Cost h;
			unsigned d;
			bool exact;
			
			Cost& prim() {return h;}
			unsigned& sec() {return d;}
		};
		
		template<typename Ign>
		struct CacheEntryImpl<false, Ign> {
			PackedState pkd;
			Cost h;
			unsigned d;
			bool exact;
			
			unsigned& prim() {return d;}
			Cost& sec() {return h;}
		};
		
		using CacheEntry = CacheEntryImpl<Minimise_Cost>;
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
				
				if(a->x() != b->x())
					return a->x() > b->x();
				
				return a->w() < b->w();
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
		
		void submitStats(int branch = -1) {
			std::string descStr = "h/d min ";
			descStr += Minimise_Cost ? "cost" : "dist";
			
			Json j;
			j["alg desc"] = descStr;
			mStatsAcc.submit(j, branch);
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
					return false;
				}
				
				bool miss = mCache.get(pkd0, ent);
					
				if(miss) {
					ent->exact = false;
					ent->sec() = Null_Sec;
	
					mAbtSearch.doSearch(s0, ent->prim());

					mStatsAcc.s_cacheMiss();
					mStatsAcc.l_cacheAdd();
				}
				else {
					mStatsAcc.s_cachePartial();
				}


				Node* n0 = mNodePool.construct();
				
				n0->pkd =		pkd0;
				n0->x() = 		0;
				n0->w() =		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;

				n0->y() = ent->prim();
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			

			Node* goalNode = nullptr;
			
			while(true) {				
				Node* n = mOpenList.pop();
					
				State s;
				mDomain.unpackState(s, n->pkd);


				if(mDomain.checkGoal(s) || n == mBestExactNode) {
					goalNode = n;
					
					if(mDomain.checkGoal(s))
						slow_assert(goalNode->y() == goalNode->x());

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
				
				if(ent->prim() < pg) {
					ent->prim() = pg;
					//mStatsAcc.l_cacheImprove();
				}
			}
			
			for(Node* n = goalNode; n != nullptr; n = n->parent) {
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				SecVal_t secVal = goalNode->w() - n->w();
				
				if(!ent->exact) {
					mStatsAcc.l_cacheMadeExact();
					slow_assert(ent->sec() == Null_Sec);
					ent->sec() = secVal;
					ent->exact = true;
				}
			}
			
			if(Minimise_Cost) {
				out_h = goalNode->y();
				out_d = goalNode->w();
			}
			else {
				out_h = goalNode->w();
				out_d = goalNode->y();
			}

			mStatsAcc.s_openListSize(mOpenList.size());
			mStatsAcc.s_closedListSize(mClosedList.getFill());
			
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mBestExactNode = nullptr;
			
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
			PrimVal_t&	kid_x		= Node::getprim(kid_g, kid_depth);
			SecVal_t&	kid_w		= Node::getsec(kid_g, kid_depth);

			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mStatsAcc.a_dups();
				if(kid_dup->x() > kid_x) {
					kid_dup->y()		-= kid_dup->x();
					kid_dup->y()		+= kid_x;
					kid_dup->x()		= kid_x;
					kid_dup->w()		= kid_w;
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
						
						if(mBestExactNode->y() > kid_dup->y()) {
							mBestExactNode = kid_dup;
							slow_assert(ent->sec() != Null_Sec);
							kid_dup->w() += ent->sec();
						}
					}
				}
			} else {
				
				Node* kid_node 		= mNodePool.construct();
				
				CacheEntry* ent = nullptr;
				
				bool miss = mCache.get(kid_pkd, ent);
			
				if(miss) {
					ent->exact = false;
					ent->sec() = Null_Sec;
					mAbtSearch.doSearch(edge.state(), ent->prim());
					mStatsAcc.l_cacheAdd();
				}

				kid_node->x() 		= kid_x;
				kid_node->w()		= kid_w;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				kid_node->y()		= kid_x + ent->prim();
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
				
				if(ent->exact && (!mBestExactNode || mBestExactNode->y() > kid_node->y())) {
					mBestExactNode = kid_node;
					slow_assert(ent->sec() != Null_Sec);
					kid_node->w() += ent->sec();
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
	
	
	template<typename D, unsigned Bound, bool Minimise_Cost, typename StatsManager>
	struct UGSAv5_Abt<D, Bound, Bound, Minimise_Cost, StatsManager> {
		
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
