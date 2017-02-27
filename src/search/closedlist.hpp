#pragma once

#include "structs/fast_hashtable.hpp"
#include "search/constants.hpp"


namespace mjon661 { namespace algorithm {
	template<typename Node_t, 
			 typename WrappedBase_t,
			 typename Pkd_t,
			 typename AlgHasher, 
			 typename AlgComp,
			 bool Use_Perfect_Hash>
	class ClosedList : public FastHashTable<
				Node_t, 
				WrappedBase_t,
				Pkd_t,
				AlgHasher, 
				AlgComp, 
				Use_Perfect_Hash>
		{
		
		using Base_t = FastHashTable<
				Node_t, 
				WrappedBase_t,
				Pkd_t,
				AlgHasher, 
				AlgComp, 
				Use_Perfect_Hash>;
		
		public:
		
		using Base_t::Wrapped_t;

		ClosedList(AlgHasher hasher, AlgComp comp) :
			Base_t(constants::Closed_List_Init_Size, hasher, comp)
		{}
	};

}}
