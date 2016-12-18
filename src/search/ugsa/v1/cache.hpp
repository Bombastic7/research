#pragma once

#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "search/ugsa/v1/common.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav1 {
	
	template<typename Domain>
	struct CacheEntry_basic {
		using PackedState = typename Domain::PackedState;
		
		CacheEntry_basic() : uh(0), exact(false) {}
		
		PackedState pkd;
		Util_t uh;
		bool exact;
	};
	
	template<typename Domain>
	struct CacheStore_basic {
		using CacheEntry = 	CacheEntry_basic<Domain>;
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
		
		CacheStore_basic(Domain const& pDomain) :
			mStore(StoreOps(pDomain), StoreOps(pDomain))
		{}
		
		CacheEntry* get(PackedState const& pkd) {
			return mStore.find(pkd);
		}
		
		bool find(PackedState const& pkd, CacheEntry*& pEnt) {
			bool b = false;
			pEnt = mStore.find(pkd);
			
			if(!pEnt) {
				b = true;
				pEnt = mPool.construct();
				pEnt->pkd = pkd;
				mStore.add(pEnt);
			}
			return b;
		}
		
		void clear() {
			mStore.clear();
			mPool.clear();
		}
		
		private:
		Store_t mStore;
		Pool_t mPool;
	};
	
}}}
