#pragma once

#include <iostream>
#include <vector>
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/time.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav2_bf {	
	
	using Util_t = double;
	
	enum struct E_StatsInfo {
		None,
		Level,
		Search,
	};
	
	enum struct E_SearchRes : unsigned {
		Search_noOpt,
		Search_opt,
		CachedSolution,
	};
	
	struct UGSAConfig {
		UGSAConfig(Json const& j);
		
		Json report() const;
		
		double wt, wf;
	};
	
	
	struct UGSABehaviour {
		static const unsigned First_Resort_Expd = 128;
		static const unsigned Next_Resort_Fact = 2;
		
		
		void reset();
		
		void update(unsigned pExpd);
		
		void putExpansion(unsigned pDelay);
		
		bool shouldUpdate(unsigned pExpd) const;
		
		double remainingExpansions(unsigned L, unsigned pDepth) const;
		
		double remainingTime(unsigned L, unsigned pDepth) const;
		
		double getExpTime() const;
		
		Json report() const;
		
		UGSABehaviour();
		

		double 		curExpTime;
		unsigned 	curExpd;

		Timer 		phaseTimer;
		unsigned 	nextExpd;
		unsigned	nUpdates;
		
		double mBF;
		std::array<unsigned, 1000> mBaseDepthCount;
		unsigned mMaxDepth;
	};
	
	
	template<typename Domain, typename CacheEntry>
	struct CacheStore_basic {

		using PackedState = typename Domain::PackedState;
		
		
		struct StoreOps {
			StoreOps(Domain const& pDomain) :
				mDomain(pDomain)
			{}
			
			size_t operator()(PackedState const& pkd) const {
				return mDomain.hash(pkd);
			}
			
			size_t operator()(CacheEntry * const ent) const {
				return mDomain.hash(ent->pkd);
			}
			
			bool operator()(CacheEntry * const ent, PackedState const& pkd) const {
				return mDomain.compare(ent->pkd, pkd);
			}
			
			Domain const& mDomain;
		};
		
		using Store_t = ClosedList<CacheEntry, 
									CacheEntry,
									PackedState, 
									StoreOps,
									StoreOps,
									Domain::Hash_Range>;

		using Pool_t = NodePool<CacheEntry, typename Store_t::Wrapped_t>;
		
		using iterator = typename Store_t::iterator;
		
		
		CacheStore_basic(Domain const& pDomain) :
			mStore(StoreOps(pDomain), StoreOps(pDomain)),
			mPool()
		{}
		
		CacheEntry* retrieve(PackedState const& pkd) {
			CacheEntry* ent = mStore.find(pkd);
			return ent;
		}
		
		bool get(PackedState const& pkd, CacheEntry*& ent) {
			bool retNew = false;
			
			ent = mStore.find(pkd);
			
			if(!ent) {
				ent = mPool.construct();
				ent->pkd = pkd;
				mStore.add(ent);
				retNew = true;
			}
			return retNew;
		}
		
		void clear() {
			mStore.clear();
			mPool.clear();
		}
		
		iterator begin() {
			return mStore.begin();
		}
		
		iterator end() {
			return mStore.end();
		}

		
		private:
		Store_t mStore;
		Pool_t mPool;

	};
	
	
	
	struct AbtStatsCounter {
		unsigned nGlobalSearches;
		
		AbtStatsCounter();
		
		unsigned getGlobalN();
		
		void reset();
	};
	
	struct NullState {};

	template<typename D, unsigned L, bool Valid>
	struct AbtStateIndirection {
		using type = typename D::template Domain<L+1>::State;
	};
	
	template<typename D, unsigned L>
	struct AbtStateIndirection<D, L, false> {
		using type = NullState;
	};
	
	template<typename D, unsigned L, bool Valid>
	struct AbstractorIndirection {
		using type = typename D::template Abstractor<L>;
	};
	
	template<typename D, unsigned L>
	struct AbstractorIndirection<D, L, false> {
		struct type {
			template<typename T>
			type(T const&) {}

			template<typename T>
			NullState operator()(T const&) {
				return NullState();
			}
		};
	};
}}}
