#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <utility>
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "search/solution.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"
#include "util/exception.hpp"
#include "util/time.hpp"
#include "search/cache_store.hpp"


namespace mjon661 { namespace algorithm { namespace bugsy {


	template<typename = void>
	struct BugsyExpAbtSearch1_common {
		template<typename Cost>
		static double compUtil(Cost cst, unsigned depth, double mCostWeight, double mDepthWeight, double mAvgBranchingFactor) {
			return mCostWeight * cst + mDepthWeight * std::pow(mAvgBranchingFactor, depth);
		}
		
		
	};

	template<typename D, unsigned L, unsigned Bound>
	class BugsyExpAbtSearch1_abt {
		public:

		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		using BaseState = typename D::template Domain<L-1>::State;
		using AbtSearch_t = BugsyExpAbtSearch1_abt<D, L+1, Bound>;
		
		struct Node {
			double ug, uf;
			PackedState pkd;
			Node* parent;
			
			Cost g;
			unsigned depth;
		};
		
		using CacheEntry = std::pair<Cost, unsigned>;
		using CacheKey = std::pair<PackedState, unsigned>;
		

		struct CacheHasher {
			CacheHasher(Domain const& pDomain) :
				mDomain(pDomain)
			{}
			
			size_t operator()(CacheKey const& k) const {
				return mDomain.hash(k.first);
			}
			
			Domain const& mDomain;
		};
		
		using CacheStore_t = std::unordered_map<CacheKey, CacheEntry, CacheHasher>;

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
		
		
		
		
		
		
		BugsyExpAbtSearch1_abt(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(10000, CacheHasher(mDomain)),
			mAbtSearch			(pDomStack, jConfig),
			mParams_expdLimit	(jConfig.count("expd_limit") ? jConfig.at("expd_limit").get<unsigned>() : (unsigned)-1)
		{
			mCostWeight = 1;
			mDistWeight = 1;
			mLog_expdThisPhase = 0;
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
			
			//mGoalNode = nullptr;
			mBestExactNode = nullptr;
		}
		
		void insertReport(Json& jReport) {
			Json j;
			j["expd_tot"] = mLog_totExpd;
			j["calls_tot"] = mLog_totCalls;
			j["searches_tot"] = mLog_totSearches;
			//j["cached_all"] = mCache.size();
			j["expd_limit"] = mParams_expdLimit;
			jReport[std::to_string(L)] = j;
			//mAbtSearch.insertReport(jReport);
		}

		void setEdgeWeights(double pCostWeight, double pDistWeight, double pBranchingFactor) {
			mCostWeight = pCostWeight;
			mDistWeight = pDistWeight;
			mBranchingFactor = pBranchingFactor;
			mCache.clear();
			
			mAbtSearch.setEdgeWeights(pCostWeight, pDistWeight, pBranchingFactor);
			
			mLog_expdThisPhase = 0;
		}
		
		std::pair<Cost, unsigned> getUtilityPath(BaseState const& pBaseState, unsigned pInitDepth) {
			mLog_totCalls++;
			
			State s;
			PackedState pkd0;
			
			s = mDomain.abstractParentState(pBaseState);
			mDomain.packState(s, pkd0);
			
			CacheKey ck{pkd0, pInitDepth};
			
			if(!mCache.count(ck)) {
				mGoalNode = nullptr;
				doSearch(s, pInitDepth);
				slow_assert(mGoalNode);
				mCache[ck] = {mGoalNode->g, mGoalNode->depth};
			}

			return mCache[ck];
		}
		
		
		void doSearch(State const& s0, unsigned pInitDepth) {
			{
				Node* n0 = mNodePool.construct();

				n0->parent = 	nullptr;
				n0->g =			0;
				n0->depth =		pInitDepth;
				n0->ug = 		BugsyExpAbtSearch1_common<>::compUtil(n0->g, n0->depth, mCostWeight, mDistWeight, mBranchingFactor);
				
				mDomain.packState(s0, n0->pkd);
				
				evalHr(n0, s0);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			

			while(true) {
				if(mLog_expd >= mParams_expdLimit)
					throw NoSolutionException(std::string("expd limit (abt ") + std::to_string(L));

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

			reset();
		}
		
		

		
		void expand(Node* n, State& s) {
			mLog_expd++;
			mLog_expdThisPhase++;
			mLog_totExpd++;
			
			typename Domain::AdjEdgeIterator edgeIt = mDomain.getAdjEdges(s);
			
			for(; !edgeIt.finished(); edgeIt.next()) {
				
				PackedState kid_pkd;
				mDomain.packState(edgeIt.state(), kid_pkd);
								
				if(n->pkd == kid_pkd)
					continue;
				
				mLog_gend++;
				
				//double kid_ug = n->ug + edgeIt.cost() * mCostWeight + mDistWeight;

				Cost kid_g = n->g + edgeIt.cost();
				unsigned kid_depth = n->depth + 1;
				double kid_ug = BugsyExpAbtSearch1_common<>::compUtil(kid_g, kid_depth, mCostWeight, mDistWeight, mBranchingFactor);
				

				Node* kid_dup = mClosedList.find(kid_pkd);

				if(kid_dup) {
					mLog_dups++;
					if(kid_dup->ug > kid_ug) {
						kid_dup->ug			= kid_ug;
						kid_dup->parent		= n;
						kid_dup->g			= kid_g;
						kid_dup->depth		= kid_depth;

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
					kid_node->g			= kid_g;
					kid_node->depth		= kid_depth;
					
					evalHr(kid_dup, edgeIt.state());
					
					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		
		
		void evalHr(Node* n, State const& s) {
			auto hrvals = mAbtSearch.getUtilityPath(s, n->depth);
			n->uf = BugsyExpAbtSearch1_common<>::compUtil(n->g + hrvals.first, n->depth + hrvals.second, mCostWeight, mDistWeight, mBranchingFactor);
		}


		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		CacheStore_t mCache;
		AbtSearch_t mAbtSearch;
		
		Node* mGoalNode, *mBestExactNode;
		double mCostWeight, mDistWeight, mBranchingFactor;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		unsigned mLog_expdThisPhase, mLog_totExpd;
		
		unsigned mLog_totCalls, mLog_totSearches;
		
		const unsigned mParams_expdLimit;
	};
	
	
	template<typename D, unsigned Bound>
	struct BugsyExpAbtSearch1_abt<D, Bound, Bound> {
		
		using BaseState = typename D::template Domain<Bound-1>::State;
		using Cost = typename D::template Domain<Bound-1>::Cost;
		
		BugsyExpAbtSearch1_abt(D&, Json const&) {}
		
		void setEdgeWeights(double, double, double) {}
		
		std::pair<Cost, unsigned> getUtilityPath(BaseState const&, unsigned) {
			return {0,0};
		}
		
		void insertReport(Json&) {}
	};
	
	
	
	
	
	
	
	
	
	
	template<typename D>
	class BugsyExpAbtSearch1 {
		public:

		using Domain = typename D::template Domain<0>;
		using Cost = typename Domain::Cost;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;

		using AbtSearch_t = BugsyExpAbtSearch1_abt<D, 1, D::Top_Abstract_Level+1>;
		
		struct Node {
			Cost g;
			double u;
			unsigned depth;
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
		
		
		

		BugsyExpAbtSearch1(D& pDomStack, Json const& jConfig) :
			mDomain				(pDomStack),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mAbtSearch			(pDomStack, jConfig),
			mParams_wf			(jConfig.at("wf")),
			mParams_wt			(jConfig.at("wt")),
			mParams_fixedExpTime(jConfig.at("fixed_exptime")),
			mParams_expdLimit	(jConfig.count("expd_limit") ? jConfig.at("expd_limit").get<unsigned>() : (unsigned)-1),
			mParams_timeLimit	(jConfig.count("time_limit") ? jConfig.at("time_limit").get<unsigned>() : 0)
		{}

		void reset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();

			mLog_expd = mLog_gend = mLog_dups = mLog_reopnd = 0;
			
			mGoalNode = nullptr;
			
			mAvgBranchingFactor = 1;

			mCostWeight = mParams_wf;
			mDistWeight = mParams_wt * mParams_fixedExpTime;
			mAbtSearch.setEdgeWeights(mCostWeight, mDistWeight, mAvgBranchingFactor);
			
			mResort_n = 0;
			mResort_next = 16;
		
			mLog_pastBf.clear();
			mLog_pastBf.push_back(mAvgBranchingFactor);
		}

		Json report() {
			Json j;
			j["expd"] = mLog_expd;
			j["gend"] = mLog_gend;
			j["dups"] = mLog_dups;
			j["reopnd"] = mLog_reopnd;
			j["wf"] = mParams_wf;
			j["wt"] = mParams_wt;
			j["cost_weight"] = mCostWeight;
			j["dist_weight"] = mDistWeight;
			
			j["fixed_exp_time"] = mParams_fixedExpTime;
			j["expd_limit"] = mParams_expdLimit;
			j["time_limit"] = mParams_timeLimit;
			
			fast_assert(mGoalNode);

			j["goal_g"] = mGoalNode->g;
			j["goal_u"] = mGoalNode->u;

			unsigned goal_depth = 0;
			for(Node* m = mGoalNode->parent; m; m=m->parent) {
				goal_depth++;
			}
				
			j["goal_depth"] = goal_depth;
			j["past_avg_bf"] = mLog_pastBf;
			
			mAbtSearch.insertReport(j);
			
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
				n0->depth =		0;
				
				evalHr(n0, s0);

				mDomain.packState(s0, n0->pkd);
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}			
			
			mLog_timer.start();
			
			while(true) {
				mLog_timer.stop();
				if(mParams_timeLimit != 0 && mLog_timer.seconds() > mParams_timeLimit) {
					throw NoSolutionException("time limit");
				}
				
				if(mLog_expd >= mParams_expdLimit)
					throw NoSolutionException("expd limit");
				
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
			mLog_expCounter[n->depth]++;
			
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
						kid_dup->depth		= n->depth + 1;
						
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
					kid_node->depth		= n->depth + 1;
					
					evalHr(kid_node, edgeIt.state());
					
					mOpenList.push(kid_node);
					mClosedList.add(kid_node);
				}
			}
		}
		
		
		void evalHr(Node* n, State const& s) {
			auto hrvals = mAbtSearch.getUtilityPath(s, 0);
			n->u = BugsyExpAbtSearch1_common<>::compUtil(n->g + hrvals.first, hrvals.second, mCostWeight, mDistWeight, mAvgBranchingFactor);
		}
		
		void doResort() {
			if(mLog_expCounter.size() > 1) {
				std::vector<double> bfsamples;
			
				auto it = mLog_expCounter.begin(), itprev = mLog_expCounter.begin();
				++it;
				
				for(; it!=mLog_expCounter.end(); ++it, ++itprev)
					bfsamples.push_back((double)it->second / itprev->second);
			
				mAvgBranchingFactor = 0;
				
				for(auto it=bfsamples.begin(); it!=bfsamples.end(); ++it)
					mAvgBranchingFactor += *it;
				
				mAvgBranchingFactor /= bfsamples.size();
			}

			mAbtSearch.setEdgeWeights(mCostWeight, mDistWeight, mAvgBranchingFactor);
			
			for(unsigned i=0; i<mOpenList.size(); i++) {
				State s;
				Node* n = mOpenList.at(i);
				mDomain.unpackState(s, n->pkd);
				evalHr(n, s);
			}
			mOpenList.reinit();
			
			mResort_n++;
			mResort_next *= 2;
			
			mLog_pastBf.push_back(mAvgBranchingFactor);
		}


		Domain mDomain;
		OpenList_t mOpenList;
		ClosedList_t mClosedList;
		NodePool_t mNodePool;
		AbtSearch_t mAbtSearch;
		
		Node* mGoalNode;
		
		std::map<unsigned, unsigned> mLog_expCounter;
		
		double mCostWeight, mDistWeight, mAvgBranchingFactor;
		
		unsigned mLog_expd, mLog_gend, mLog_dups, mLog_reopnd;
		
		unsigned mResort_next, mResort_n;
		
		const double mParams_wf, mParams_wt, mParams_fixedExpTime;
		const unsigned mParams_expdLimit, mParams_timeLimit;
		CpuTimer mLog_timer;
		
		std::vector<double> mLog_pastBf;
	};
	
	
}}}
