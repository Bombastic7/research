#pragma once

#include <string>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"
#include "search/cache_store.hpp"


namespace mjon661 { namespace algorithm { namespace bugsy {



	template<typename D>
	class BugsyLinearAbtSearch {
		public:

		using Domain = typename D::template Domain<1>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		
		struct Node {
			double ug, uf;
			PackedState pkd;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			bool exact;
			double uh;
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
				if(a->uf != b->uf)
					return a->uf < b->uf;
				return a->ug > b->ug;
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
		
		
		
		
		
		
		BugsyLinearAbtSearch(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			()
		{
			mCostWeight = 1;
			mDistWeight = 1;
		}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;
			mBestExactNode = nullptr;
		}

		void setEdgeWeights(double pCostWeight, double pDistWeight) {
			mCostWeight = pCostWeight;
			mDistWeight = pDistWeight;
			mCache.clear();
		}
		
		double getUtility(BaseState const& pBaseState) {
			State s;
			PackedState pkd0;
			
			s = mDomain.abstractParentState(pBaseState);
			mDomain.packState(s, pkd0);
			
			CacheEntry* ent = mCache.retrieve(pkd0);
			if(!ent) {
				doSearche(s);
				ent = mCache.retrieve(pkd0);
				slow_assert(ent && ent->exact);
			}

			return ent->uh;
		}
		
		void execute(State const& s0) {
			PackedState pkd0;
			mDomain.packState(s0, pkd0);
			
			CacheEntry* ent = mCache.retrieve(pkd0)
			if(ent && ent->exact) {
				return ent->uh;
			}
			
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->ug = 		0;
				n0->parent = 	nullptr;

				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			

			while(true) {
				Node* n = nullptr;
				try {
					n = mOpenList.pop();
				}
				catch(AssertException const& e) {
					throw NoSolutionException("");
				}
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mBestExactNode == n || mDomain.checkGoal(s)) {
					mGoalNode = n;
					break;
				}
				
				expand(n, s);
			}
		}
		
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;

			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				double kid_ug = n->ug + edgeIt.cost() * mCostWeight + mDistWeight;

				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->ug > kid_ug) {
						kid_dup->ug			= kid_ug;
						kid_dup->parent		= n;
						
						evalHr(kid_dup, edgeIt.state());

						if(!mOpenList.contains(kid_dup)) {
							mLog_reopnd++;
						}

						mOpenList.pushOrUpdate(kid_dup);
					}
				}
				else {
					Node* kid_node 		= mNodePool.construct();

					kid_node->ug 		= kid_ug;
					kid_node->pkd 		= kid_pkd;
					kid_node->parent	= n;

					evalHr(kid_node, edgeIt.state());
					
					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		
		
		void evalHr(Node* n, State const& s) {
			CacheEntry* ent;
			
			bool newEntry = mCache.get(n->pkd, ent);
			
			if(newEntry) {
				ent->exact = false;
				ent->uh = 0;
			}
			
			n->uf = n->ug + ent->uh;
			
			if(ent->exact) {
				if(!mBestExactNode || mBestExactNode->uf > n->uf)
					mBestExactNode = n;
			}
		}
		
		
		void doCaching() {
			slow_assert(mGoalNode);
			
			for(auto it=mClosedList.begin(); it!=mClosedList.begin(); ++it) {
				Node* n = *it;
				
				double pghr = mGoalNode->uf - n->ug;
				
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				if(pghr > ent->uh)
					ent->uh = pghr;
			}
			
			for(Node* n=mGoalNode; n; n=n->parent) {
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				if(!ent->exact)
					ent->exact = true;
			}
			
		}
		

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		CacheStore_t mCache;
		
		Node* mGoalNode, *mBestExactNode;
		double mCostWeight, mDistWeight;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
	};
	
	
	
	
	
	
	
	
	
	
	
	
	
	template<typename D>
	class BugsyAbtSearchBase {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		using AbtSearch_t = BugsyLinearAbtSearch<D>;
		
		struct Node {
			Cost g;
			double u;
			PackedState pkd;
			Node* parent;
			unsigned expdGen;
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
				if(a->u != b->u)
					return a->u < b->u;
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
		
		
		

		BugsyAbtSearchBase(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mAbtSearch			(pDomStack, jConfig),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt")),
			mParams_fixedExpTime(jConfig.at("fixed_exp_time"))
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;
			
			mAvgDelay = 1;
			mDistWeight = mParams_wt * mParams_fixedExpTime * mAvgDelay;
			mAbtSearch.setWeights(mParams_wf, mDistWeight);
			
			mResort_n = 0;
			mResort_next = 16;
		}

		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["dups"] = mLog_dups;
			j["reopnd"] = mLog_reopnd;
			j["wf"] = mParams_wf;
			j["wt"] = mParams_wt;
			j["fixed_exp_time"] = mParams_fixedExpTime;
			
			fast_assert(mGoalNode);

			j["goal_g"] = mGoalNode->g;
			j["goal_f"] = mGoalNode->f;
				
			unsigned goal_depth = 0;
			for(Node* m = mGoalNode->parent; m; m=m->parent) {
				goal_depth++;
			}
				
			j["goal_depth"] = goal_depth;
			
			return j;
		}
		
		void execute(State const& s0) {
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		0;
				n0->parent = 	nullptr;
				n0->expdGen =	0;
				
				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			

			while(true) {
				if(mLog_expd == mResort_next) {
					doResort();
				}
				
				Node* n = nullptr;
				try {
					n = mOpenList.pop();
				}
				catch(AssertException const& e) {
					throw NoSolutionException("");
				}
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					mGoalNode = n;
					break;
				}
				
				expand(n, s);
			}
		}		

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			mDelayAcc += mLog_expd - n->expdGen;
			
			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				Cost kid_g = n->g + edgeIt.cost();
				
				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->g > kid_g) {
						kid_dup->g			= kid_g;
						kid_dup->parent		= n;
						kid_dup->expdGen	= mLog_expd;
						
						evalHr(kid_dup, edgeIt.state());

						if(!mOpenList.contains(kid_dup)) {
							mLog_reopnd++;
						}

						mOpenList.pushOrUpdate(kid_dup);
					}
				}
				else {
					Node* kid_node 		= mNodePool.construct();

					kid_node->g 		= kid_g;
					kid_node->pkd 		= kid_pkd;
					kid_node->parent	= n;
					kid_node->expdGen	= mLog_expd;
					
					evalHr(kid_node, edgeIt.state());
					
					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		
		
		void evalHr(Node* n, State const& s) {			
			n->u = n->g * mParams_wf + mAbtSearch.getUtility(s);
		}
		
		void doResort() {
			unsigned expThisPhase = mResort_n == 0 ? 16 : mLog_expd / 2;
			
			mAvgDelay = (double)mDelayAcc / expThisPhase;
			mDelayAcc = 0;
			
			mDistWeight = mParams_wt * mParams_fixedExpTime * mAvgDelay;
			mAbtSearch.setWeights(mParams_wf, mAvgDelay);
			
			for(unsigned i=0; i<mOpenList.size(); i++) {
				State s;
				Node* n = mOpenList.at(i);
				mDomain.unpackState(s, n->pkd);
				evalHr(n, s);
			}
			mOpenList.reinit();
			
			mResort_n++;
			mResort_next *= 2;
			
			mLog_curDistFact = mLog_curExpTime * mParams_wt;
		}


		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		AbtSearch_t mAbtSearch;
		
		Node* mGoalNode, *mBestExactNode;
		double mAvgDelay, mDistWeight;
		unsigned mDelayAcc;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		unsigned mResort_next, mResort_n;
		
		const double mParams_wf, mParams_wt, mParams_fixedExpTime;
	};
	
	
}}}
