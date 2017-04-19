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



	template<typename D, unsigned L, unsigned Bound>
	class BugsyAbtLin_abstractSearch {
		public:

		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		using BaseState = typename D::template Domain<L-1>::State;
		using AbtSearch_t = BugsyAbtLin_abstractSearch<D, L+1, Bound>;
		
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
		
		
		
		
		
		
		BugsyAbtLin_abstractSearch(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain),
			mAbtSearch			(pDomStack, jConfig)
		{
			resetAll();
		}

		void resetAll() {
			reset();
			
			mPrin_costWeight = mPrin_distWeight = 1;
			mPrin_phaseExpd = 0;
			mPrin_calls = 0;
			mPrin_searches = 0;
			mPrin_pastPhaseExpds.clear();
			mPrin_pastCacheSz.clear();
		}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mGoalNode = nullptr;
			mBestExactNode = nullptr;
		}
		
		void insertReport(Json& jReport) {
			std::vector<unsigned> pastExpd = mPrin_pastPhaseExpds;
			pastExpd.push_back(mPrin_phaseExpd);
			
			unsigned totExpd = 0;
			for(auto i : pastExpd)
				totExpd += i;
			
			std::vector<unsigned> pastCacheSz = mPrin_pastCacheSz;
			pastCacheSz.push_back(mCache.size());
			
			Json j;
			j["tot_expd"] = totExpd;
			j["calls"] = mPrin_calls;
			j["searches"] = mPrin_searches;
			j["phase_expd"] = pastExpd;
			j["cache_size"] = pastCacheSz;
			jReport[std::to_string(L)] = j;
			
			mAbtSearch.insertReport(jReport);
		}

		void setEdgeWeights(double pCostWeight, double pDistWeight) {
			mPrin_costWeight = pCostWeight;
			mPrin_distWeight = pDistWeight;
			
			mPrin_pastPhaseExpds.push_back(mPrin_phaseExpd);
			mPrin_phaseExpd = 0;
			
			mPrin_pastCacheSz.push_back(mCache.size());
			mCache.clear();
			
			mAbtSearch.setEdgeWeights(pCostWeight, pDistWeight);
		}
		
		double getAbstractUtility(BaseState const& pBaseState) {
			mPrin_calls++;
			
			State s;
			PackedState pkd0;
			
			s = mDomain.abstractParentState(pBaseState);
			mDomain.packState(s, pkd0);
			
			CacheEntry* ent = mCache.retrieve(pkd0);
			if(!ent) {
				mPrin_searches++;
				doSearch(s);
				ent = mCache.retrieve(pkd0);
				slow_assert(ent);
				slow_assert(ent->exact);
			}

			return ent->uh;
		}
		

		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->ug = 		0;
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
		}
		
		

		
		void expand(Node* n, State& s) {
			mPrin_phaseExpd++;

			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				//mLog_gend++;
				
				double kid_ug = n->ug + edgeIt.cost() * mPrin_costWeight + mPrin_distWeight;

				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					//mLog_dups++;
					if(kid_dup->ug > kid_ug) {
						kid_dup->ug			= kid_ug;
						kid_dup->parent		= n;
						
						evalHr(kid_dup, edgeIt.state());

						//~ if(!mOpenList.contains(kid_dup)) {
							//~ mLog_reopnd++;
						//~ }

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
				ent->uh = mAbtSearch.getAbstractUtility(s);
			}
			
			n->uf = n->ug + ent->uh;
			
			if(ent->exact) {
				if(!mBestExactNode || mBestExactNode->uf > n->uf)
					mBestExactNode = n;
			}
		}
		
		
		void doCaching() {
			slow_assert(mGoalNode);
			
			for(auto it=mClosedList.begin(); it!=mClosedList.end(); ++it) {
				Node* n = *it;
				
				if(mOpenList.contains(n))
					continue;
				
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
		AbtSearch_t mAbtSearch;
		
		Node* mGoalNode, *mBestExactNode;
		double mPrin_costWeight, mPrin_distWeight;

		unsigned mPrin_phaseExpd, mPrin_calls, mPrin_searches;
		
		std::vector<unsigned> mPrin_pastPhaseExpds;
		std::vector<unsigned> mPrin_pastCacheSz;
	};
	
	
	template<typename D, unsigned Bound>
	struct BugsyAbtLin_abstractSearch<D, Bound, Bound> {
		
		using BaseState = typename D::template Domain<Bound-1>::State;
		
		BugsyAbtLin_abstractSearch(D&, Json const&) {}
		
		void setEdgeWeights(double, double) {}
		
		double getAbstractUtility(BaseState const&) {
			return 0;
		}
		
		void insertReport(Json&) {}
	};
	
	
	
	
	

	template<typename D, bool OP_Static_Exp_Time>
	class BugsyAbtLin_baseSearch {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		
		struct Node {
			Cost g;
			double u;
			unsigned expdAtGen;
			PackedState pkd;
			Node* parent;
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
		
		
		
		
		
		
		BugsyAbtLin_baseSearch(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mAbtSearch			(pDomStack, jConfig),
			mParam_wf			(jConfig.at("wf")),
			mParam_wt			(jConfig.at("wt")),
			mParam_stExpTime	(OP_Static_Exp_Time ? jConfig.at("exptime").get<double>() : 0)
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;		

			mLog_curDelay = 0;
			mLog_nextDelayAcc = 0;
			mResort_n = 0;
			mResort_next = 16;

			if(OP_Static_Exp_Time)
				mLog_curExpTime = mParam_stExpTime;
			else
				mLog_curExpTime = 0;
			
			mLog_searchTimer.start();
			mLog_resortTimer.start();
			
			mLog_curDistFact = mParam_wt * mLog_curDelay * mLog_curExpTime;
			
			mLog_pastDelays.clear();
			mLog_pastExpTimes.clear();
			
			mLog_pastExpTimes.push_back(mLog_curExpTime);
			mLog_pastDelays.push_back(mLog_curDelay);
			
			mAbtSearch.setEdgeWeights(mParam_wf, mLog_curDistFact);
		}
		
		void execute(State const& s0) {
			doSearch(s0);
		}
		
		void doSearch(State const& s0) {
			reset();
			{
				Node* n0 = mNodePool.construct();

				n0->g = 		Cost(0);
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

				if(mDomain.checkGoal(s)) {
					mGoalNode = n;
					break;
				}
				
				expand(n, s);
				
				if(mLog_expd == mResort_next)
					doResort();
			}
			mLog_searchTimer.stop();
		}
		
		void doResort() {
			unsigned expThisPhase = mResort_n == 0 ? 16 : mLog_expd / 2;

			mLog_curDelay = (double)mLog_nextDelayAcc / expThisPhase;
			mLog_nextDelayAcc = 0;

			if(!OP_Static_Exp_Time) {
				mLog_resortTimer.stop();
				mLog_resortTimer.start();
				mLog_curExpTime = mLog_resortTimer.cpuSeconds() / expThisPhase;
			}

			mLog_curDistFact = mParam_wt * mLog_curDelay * mLog_curExpTime;

			for(unsigned i=0; i<mOpenList.size(); i++) {
				State s;
				Node* n = mOpenList.at(i);
				mDomain.unpackState(s, n->pkd);
				evalHr(n, s);
			}
			mOpenList.reinit();
			
			mResort_n++;
			mResort_next *= 2;
			
			mLog_pastExpTimes.push_back(mLog_curExpTime);
			mLog_pastDelays.push_back(mLog_curDelay);
		
			mAbtSearch.setEdgeWeights(mParam_wf, mLog_curDistFact);
		}
		
		
		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["reopnd"] = mLog_reopnd;
			j["dups"] = mLog_dups;
			j["wf"] = mParam_wf;
			j["wt"] = mParam_wt;
			
			if(OP_Static_Exp_Time)
				j["fixed_exp_time"] = mParam_stExpTime;
			
			j["wall_time"] = mLog_searchTimer.wallSeconds();
			j["cpu_time"] = mLog_searchTimer.cpuSeconds();
			
			fast_assert(mGoalNode);
			
			j["goal_g"] = mGoalNode->g;
			
			unsigned goal_depth = 0;
			for(Node* m = mGoalNode->parent; m; m=m->parent) {
				goal_depth++;
			}
			
			j["goal_depth"] = goal_depth;

			j["past_delays"] = mLog_pastDelays;
			j["past_exptimes"] = mLog_pastExpTimes;
			
			j["abt"] = Json();
			mAbtSearch.insertReport(j.at("abt"));
			
			return j;
		}
		
		
		//private:
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			//logDebugStream() << mLog_expd << ": " << n->u << "\n";
			
			slow_assert(n->expdAtGen < mLog_expd);
			mLog_nextDelayAcc += mLog_expd - n->expdAtGen;
			
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
						kid_dup->expdAtGen	= mLog_expd;
						
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
					kid_node->expdAtGen	= mLog_expd;
					
					evalHr(kid_node, edgeIt.state());

					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		

		void evalHr(Node* n, State const& s) {		
			n->u = mParam_wf * n->g + mAbtSearch.getAbstractUtility(s);
		}
		
	
		
		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		BugsyAbtLin_abstractSearch<D, 1, D::Top_Abstract_Level+1> mAbtSearch;
		
		const double mParam_wf, mParam_wt, mParam_stExpTime;
		
		Node* mGoalNode;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		double mLog_curDistFact, mLog_curExpTime, mLog_curDelay;
		unsigned mResort_n, mResort_next, mLog_nextDelayAcc;
		
		std::vector<double> mLog_pastDelays, mLog_pastExpTimes;
		
		Timer mLog_resortTimer, mLog_searchTimer;
	};
	
}}
