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


namespace mjon661 { namespace algorithm {



	template<typename D, unsigned L, unsigned Bound, bool Min_Cost>
	class AdmissibleAbtSearch {
		public:

		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		using BaseState = typename D::template Domain<L-1>::State;
		using AbtSearch_t = AdmissibleAbtSearch<D, L+1, Bound, Min_Cost>;
		
		struct Node {
			Cost g, f;
			PackedState pkd;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			bool exact;
			Cost h;
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
		
		
		
		
		
		
		AdmissibleAbtSearch(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain),
			mAbtSearch			(pDomStack, jConfig)
		{
			mLog_totExpd = 0;
			mLog_totCalls = 0;
			mLog_totSearches = 0;
			reset();
		}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;
			mBestExactNode = nullptr;
		}
		
		void insertReport(Json& jReport) {
			Json j;
			j["expd_tot"] = mLog_totExpd;
			j["calls_tot"] = mLog_totCalls;
			j["searches_tot"] = mLog_totSearches;
			j["cached_all"] = mCache.size();
			
			unsigned exactCached = 0;
			
			for(auto it=mCache.begin(); it != mCache.end(); ++it) {
				if((*it)->exact)
					exactCached++;
			}
			
			j["cached_exact"] = exactCached;
			
			jReport[std::to_string(L)] = j;
			mAbtSearch.insertReport(jReport);
		}


		
		double getHrVal(BaseState const& pBaseState) {
			mLog_totCalls++;			
			
			State s;
			PackedState pkd0;
			
			s = mDomain.abstractParentState(pBaseState);
			mDomain.packState(s, pkd0);
			
			CacheEntry* ent = mCache.retrieve(pkd0);
			if(!ent || !ent->exact) {
				mLog_totSearches++;
				doSearch(s);
				ent = mCache.retrieve(pkd0);
				slow_assert(ent);
				slow_assert(ent->exact);
			}
			
			return ent->h;
		}

		
		void doSearch(State const& s0) {
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		0;
				n0->parent = 	nullptr;

				mDomain.packState(s0, n0->pkd);
				
				evalHr(n0, s0);
				
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
			
			doCaching();
			reset();
		}
		
		void expand(Node* n, State& s) {
			mLog_expd++;
			mLog_totExpd++;

			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				Cost kid_g = Min_Cost ? n->g + edgeIt.cost() : n->g + 1;

				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->g > kid_g) {
						kid_dup->g			= kid_g;
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

					kid_node->g 		= kid_g;
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
				ent->h = mAbtSearch.getHrVal(s);
			}
			
			n->f = n->g + ent->h;
			
			if(ent->exact) {
				if(!mBestExactNode || mBestExactNode->f > n->f)
					mBestExactNode = n;
			}
		}
		
		
		void doCaching() {
			slow_assert(mGoalNode);
			
			for(auto it=mClosedList.begin(); it!=mClosedList.end(); ++it) {
				Node* n = *it;
				
				if(mOpenList.contains(n))
					continue;
				
				double pghr = mGoalNode->f - n->g;
				
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				if(pghr > ent->h)
					ent->h = pghr;
			}
			
			for(Node* n=mGoalNode; n; n=n->parent) {
				CacheEntry* ent = mCache.retrieve(n->pkd);
				slow_assert(ent);
				
				if(!ent->exact) {
					
					//~ {
						//~ State s;
						//~ mDomain.unpackState(s, n->pkd);
						//~ logDebugStream() << "Made exact (" << L << "): ";
						//~ mDomain.prettyPrintState(s, g_logDebugOfs);
						//~ g_logDebugOfs << " " << ent->h << "\n";
					//~ }
					ent->exact = true;
				}
			}
			
		}
		

		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		CacheStore_t mCache;
		AbtSearch_t mAbtSearch;
		
		Node* mGoalNode, *mBestExactNode;
		double mCostWeight, mDistWeight;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		unsigned mLog_totExpd;
		
		unsigned mLog_totCalls, mLog_totSearches;
	};
	
	
	template<typename D, unsigned Bound, bool Min_Cost>
	struct AdmissibleAbtSearch<D, Bound, Bound, Min_Cost> {
		
		using BaseState = typename D::template Domain<Bound-1>::State;
		
		AdmissibleAbtSearch(D&, Json const&) {}
		
		double getHrVal(BaseState const&) {
			return 0;
		}
		
		void insertReport(Json&) {}
	};
	

	
}}
