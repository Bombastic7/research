#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "search/hastar/generic/common.hpp"
#include "search/closedlist.hpp"
#include "search/openlist.hpp"
#include "search/nodepool.hpp"
#include "util/debug.hpp"
#include "util/json.hpp"


namespace mjon661 { namespace algorithm { namespace hastargeneric {

	template<	typename D,
				unsigned L,
				unsigned Bound,
				E_StatsInfo P_Stats_Info>
	
	class HAstar_Abt {
		
		static const bool St_Col_Level = P_Stats_Info >= E_StatsInfo::Level;
		static const bool St_Col_Search = P_Stats_Info >= E_StatsInfo::Search;
		
		
		using Domain = typename D::template Domain<L>;
		using Cost = typename Domain::Cost;
		using Operator = typename Domain::Operator;
		using OperatorSet = typename Domain::OperatorSet;
		using State = typename Domain::State;
		using PackedState = typename Domain::PackedState;
		using Edge = typename Domain::Edge;
		using Abstractor = typename AbstractorIndirection<D, L, L < Bound-1>::type;
		using AbtState = typename AbtStateIndirection<D, L, L < Bound-1>::type;
		
		using NextAbtSearch = HAstar_Abt<D, L+1, Bound, P_Stats_Info>;
		
		struct Node {
			Cost g, f;
			PackedState pkd;
			Operator in_op;
			Operator parent_op;
			Node* parent;
		};
		
		struct CacheEntry {
			PackedState pkd;
			bool exact;
			Cost h;
		};
		using CacheStore = CacheStore_basic<Domain, CacheEntry>;
		
		struct SearchStats {
			
			void inc_expd() { if(St_Col_Level) expd++; } //Changed to level so we could see total expd.
			void inc_gend() { if(St_Col_Search) gend++; }
			void inc_dups() { if(St_Col_Search) dups++; }
			void inc_reopnd() { if(St_Col_Search) reopnd++; }
			
			void set_resolution(E_SearchRes r) {
				if(St_Col_Search)
					res = (unsigned)r;
			}
			
			void set_global(AbtStatsCounter& pCounter) {
				if(St_Col_Search)
					globalN = pCounter.getGlobalN();
			}
			
			void set_local(unsigned i) {
				if(St_Col_Search)
					localN = i;
			}
			
			void reset() {
				expd = gend = dups = reopnd = 0;
				res = globalN = localN = 0;
			}
			
			unsigned expd, gend, dups, reopnd;
			unsigned res;
			unsigned globalN, localN;
		};
		
		struct LevelStats {
			
			void resetSearches() {
				mSearchStatsCollection.clear();
				nLocalSearches = 0;
				mTotExpd = 0;
			}
			
			void resetCache() {
				nTotalCached = nExactCached = 0;
			}
			
			void pushSearch(SearchStats const& pStats) {
				if(St_Col_Search)
					mSearchStatsCollection.push_back(pStats);
				
				if(St_Col_Level)
					mTotExpd += pStats.expd;
			}
			
			unsigned getLocalN() {
				if(St_Col_Search)
					return nLocalSearches++;
				return 0;
			}
			
			void inc_nTotalCached() { if(St_Col_Level) nTotalCached++; }
			void inc_nExactCached() { if(St_Col_Level) nExactCached++; }
			void inc_nLocalSearches() { if(St_Col_Level) nLocalSearches++; }
			
			LevelStats() {
				resetSearches();
				resetCache();
			}
			
			Json dumpLevelStats() {
				Json j;
				j["nTotalCached"] = nTotalCached;
				j["nExactCached"] = nExactCached;
				j["nLocalSearches"] = nLocalSearches;
				j["total expd"] = mTotExpd;
				return j;
			}
			
			void dumpSearches(std::ostream& out) {
				out << "expd " << "gend " << "dups " << "reopnd ";
				out << "res ";
				out << "globalN " << "localN";
				out << "\n";
				
				for(SearchStats const& s : mSearchStatsCollection) {
					out << s.expd << " " << s.gend << " " << s.dups << " " << s.reopnd << " ";
					out << s.res << " ";
					out << s.globalN << " " << s.localN;
					out << "\n";
				}
			}
			
			std::vector<SearchStats> mSearchStatsCollection;
			unsigned nTotalCached, nExactCached;
			unsigned nLocalSearches;
			unsigned mTotExpd;
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
		
		
		public:

		HAstar_Abt(D& pDomStack, AbtStatsCounter& pGlobalStats) :
			mSearchStats		(),
			mLevelStats			(),
			mGlobalStats		(pGlobalStats),
			mAbtSearch			(pDomStack, pGlobalStats),
			mAbtor				(pDomStack),
			mDomain				(pDomStack),
			mOptNode			(nullptr),
			mOpenList			(OpenOps()),
			mClosedList			(ClosedOps(mDomain), ClosedOps(mDomain)),
			mNodePool			(),
			mCache				(mDomain)
		{}


		Cost getPathCost(State const& s0) {
			mSearchStats.set_global(mGlobalStats);
			mSearchStats.set_local(mLevelStats.getLocalN());
			
			Cost retCost;
			
			PackedState pkd;
			mDomain.packState(s0, pkd);
			CacheEntry* ent = mCache.retrieve(pkd);
			
			if(ent && ent->exact) {
				mSearchStats.set_resolution(E_SearchRes::CachedSolution);
				retCost = ent->h;
			} else {
			
				Node* n = doSearch(s0);
				retCost = n->f;
				
				mLevelStats.pushSearch(mSearchStats);
				
				searchReset();
			}
			return retCost;
		}
		
		void levelResetRec() {
			mCache.clear();
			mLevelStats.resetCache();
			mLevelStats.resetSearches();
			searchReset();
			mAbtSearch.levelResetRec();
		}
		
		void levelResetRec_preserveCache() {
			mLevelStats.resetSearches();
			searchReset();
			mAbtSearch.levelResetRec_preserveCache();
		}
		
		void addToReportRec(Json& jReport) {
			std::string key = std::string("Level ") + std::to_string(L);
			
			jReport[key] = mLevelStats.dumpLevelStats();
			mAbtSearch.addToReportRec(jReport);
		}
		
		void dumpSearchesRec(std::string const& pPrefix) {
			std::ofstream ofs(pPrefix + std::to_string(L));
			
			if(ofs) {
				mLevelStats.dumpSearches(ofs);
			} else {
				std::string logMsg(__FILE__ " : ");
				logMsg.append(std::to_string(__LINE__) + " : ");
				logMsg.append("Level ").append(std::to_string(L)).append(" : ");
				logMsg.append("Could not open dump file");
				logDebug(logMsg.c_str());
			}
			mAbtSearch.dumpSearchesRec(pPrefix);
		}
		
		
		private:
		
		Node* doSearch(State const& s0) {			
			{
				Node* n0 = mNodePool.construct();
				
				n0->g = 		0;
				n0->in_op = 	mDomain.noOp;
				n0->parent_op = mDomain.noOp;
				n0->parent = 	nullptr;
				
				mDomain.packState(s0, n0->pkd);
				
				CacheEntry* ent = getCacheEntry(s0, n0->pkd);
				n0->f = ent->h;
				
				mOpenList.push(n0);
				mClosedList.add(n0);
			}
			
			Node* retNode = nullptr;
			while(true) {				
				Node* n = mOpenList.pop();
				
				if(n == mOptNode) {
					retNode = n;
					mSearchStats.set_resolution(E_SearchRes::Search_opt);
					break;
				}
					
				State s;
				mDomain.unpackState(s, n->pkd);

				if(mDomain.checkGoal(s)) {
					retNode = n;
					mSearchStats.set_resolution(E_SearchRes::Search_noOpt);
					break;
				}
				
				expand(n, s);
			}
			
			cacheSearchResults(retNode);
			
			return retNode;
		}
		
		void searchReset() {
			mOpenList.clear();
			mClosedList.clear();
			mNodePool.clear();
			mSearchStats.reset();
			mOptNode = nullptr;
		}
		
		void expand(Node* n, State& s) {
			mSearchStats.inc_expd();
			
			OperatorSet ops = mDomain.createOperatorSet(s);
			
			for(unsigned i=0; i<ops.size(); i++) {
				if(ops[i] == n->parent_op)
					continue;
				
				mSearchStats.inc_gend();
				considerkid(n, s, ops[i]);
			}
		}
		
		void considerkid(Node* pParentNode, State& pParentState, Operator const& pInOp) {

			Edge		edge		= mDomain.createEdge(pParentState, pInOp);
			Cost		kid_g		= pParentNode->g + edge.cost();
			
			PackedState kid_pkd;
			mDomain.packState(edge.state(), kid_pkd);

			Node* kid_dup = mClosedList.find(kid_pkd);

			if(kid_dup) {
				mSearchStats.inc_dups();
				if(kid_dup->g > kid_g) {
					kid_dup->g			= kid_g;
					kid_dup->in_op		= pInOp;
					kid_dup->parent_op 	= edge.parentOp();
					kid_dup->parent	   	= pParentNode;
					
					kid_dup->f		   	-= kid_dup->g;
					kid_dup->f		   	+= kid_g;
					
					if(!mOpenList.contains(kid_dup))
						mSearchStats.inc_reopnd();
					
					CacheEntry* ent = getCacheEntry_noMiss(kid_pkd);
					
					if(ent->exact) {
						slow_assert(mOptNode);
						if(kid_dup->f < mOptNode->f)
							mOptNode = kid_dup;
					}
					
					mOpenList.pushOrUpdate(kid_dup);
				}
			} else {
				Node* kid_node 		= mNodePool.construct();
				CacheEntry* ent 	= getCacheEntry(edge.state(), kid_pkd);

				kid_node->g 		= kid_g;
				kid_node->f 		= kid_g + ent->h;
				kid_node->pkd 		= kid_pkd;
				kid_node->in_op 	= pInOp;
				kid_node->parent_op = edge.parentOp();
				kid_node->parent	= pParentNode;
				
				if(ent->exact) {
					if(!mOptNode || kid_node->f < mOptNode->f)
						mOptNode = kid_node;
				}
				
				mOpenList.push(kid_node);
				mClosedList.add(kid_node);
			}
			
			mDomain.destroyEdge(edge);
		}
		
		CacheEntry* getCacheEntry(State const& s, PackedState const& pkd) {
			CacheEntry* ent;
			bool newEnt = mCache.get(pkd, ent);
			
			if(newEnt) {
				mLevelStats.inc_nTotalCached();
				AbtState abtState = mAbtor(s);
				ent->h = mAbtSearch.getPathCost(abtState);
				ent->exact = false;
			}
			return ent;
		}
		
		CacheEntry* getCacheEntry_noMiss(PackedState const& pkd) {
			CacheEntry* ent;
			bool newEnt = mCache.get(pkd, ent);
			slow_assert(!newEnt);
			return ent;
		}
		
		
		void cacheSearchResults(Node* goalNode) {
			using It_t = typename ClosedList_t::iterator;
			
			for(It_t it = mClosedList.begin(); it != mClosedList.end(); ++it) {
				Node* n = *it;
				CacheEntry* ent = getCacheEntry_noMiss(n->pkd);
				
				if(ent->exact)
					continue;
				
				Cost pg = goalNode->f - n->g;
				
				if(ent->h < pg) {
					ent->h = pg;
				}
			}
			
			for(Node* n = goalNode; n; n = n->parent) {
				CacheEntry* ent = getCacheEntry_noMiss(n->pkd);
				
				if(!ent->exact)
					mLevelStats.inc_nExactCached();
				
				ent->exact = true;
			}
		}
		
		
		SearchStats 		mSearchStats;
		LevelStats			mLevelStats;
		AbtStatsCounter&	mGlobalStats;
		
		NextAbtSearch		mAbtSearch;
		Abstractor			mAbtor;
		
		const Domain		mDomain;
		Node*				mOptNode;
		
		OpenList_t 			mOpenList;
		ClosedList_t 		mClosedList;
		NodePool_t 			mNodePool;
		CacheStore			mCache;
	};
	
	
	template<typename D, unsigned Bound, E_StatsInfo P_Stats_Info>
	class HAstar_Abt<D, Bound, Bound, P_Stats_Info> {
		
		using Cost = typename D::template Domain<Bound - 1>::Cost;
		
		public:
		
		HAstar_Abt(D& pDomStack, AbtStatsCounter& pGlobalStats) {
			
		}
		
		Cost getPathCost(NullState const&) {
			return 0;
		}
		
		void levelResetRec() {}
		void levelResetRec_preserveCache() {}
		void addToReportRec(Json&) {}
		void dumpSearchesRec(std::string const&) {}

	};
}}}
