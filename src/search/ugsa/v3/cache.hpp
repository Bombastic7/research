#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include "search/closedlist.hpp"
#include "search/nodepool.hpp"
#include "util/exception.hpp"
#include "util/json.hpp"
#include "util/debug.hpp"
#include "util/time.hpp"

#include "search/ugsa/v3/defs.hpp"


namespace mjon661 { namespace algorithm { namespace ugsav3 {

	template<typename Domain, typename CacheEntry>
	struct CacheStore {

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
		
		
		CacheStore(Domain const& pDomain) :
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
	
}}}
