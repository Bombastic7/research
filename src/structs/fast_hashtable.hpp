#pragma once

#include "structs/fast_hashtable_perfect.hpp"
#include "structs/fast_hashtable_imperfect.hpp"

#include "util/debug.hpp"


namespace mjon661 {


	template<typename Elm_t, 
		 typename WrappedBase_t, 
		 typename Key_t, 
		 typename Hasher, 
		 typename Comparer,
		 bool Perfect_Hash>
	class FastHashTable;
	
	template<typename Elm_t, 
		 typename WrappedBase_t, 
		 typename Key_t, 
		 typename Hasher,
		 typename Comparer>
	class FastHashTable<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, true>
		: public FastHashTablePerfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>
	{
		using FastHashTablePerfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>::FastHashTablePerfect;
	};
	
	template<typename Elm_t, 
		 typename WrappedBase_t, 
		 typename Key_t, 
		 typename Hasher, 
		 typename Comparer>
	class FastHashTable<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, false>
		: public FastHashTableImperfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>
	{
		using FastHashTableImperfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>::FastHashTableImperfect;
	};

}
