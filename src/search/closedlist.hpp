#pragma once

#include "structs/fast_hashtable.hpp"
#include "search/constants.hpp"


namespace mjon661 { namespace algorithm {
	template<typename Node_t, 
			 typename WrappedBase_t,
			 typename Pkd_t,
			 typename AlgHasher, 
			 typename AlgComp,
			 size_t Hash_Range>
	class ClosedList : public FastHashTable<
				Node_t, 
				WrappedBase_t,
				Pkd_t,
				AlgHasher, 
				AlgComp, 
				Hash_Range < constants::Max_Perfect_Hash>
		{
		
		using Base_t = FastHashTable<
				Node_t, 
				WrappedBase_t,
				Pkd_t,
				AlgHasher, 
				AlgComp, 
				Hash_Range < constants::Max_Perfect_Hash>;
		
		public:
		
		using Base_t::Wrapped_t;
		
		static const unsigned Init_Size = Hash_Range < constants::Max_Perfect_Hash ? 
														Hash_Range : constants::Closed_List_Init_Size;
		
		ClosedList(AlgHasher hasher, AlgComp comp) :
			Base_t(Init_Size, hasher, comp)
		{}
	};

}}
