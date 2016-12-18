#pragma once

#include "structs/fast_hashtable_perfect.hpp"
#include "structs/fast_hashtable_imperfect.hpp"


namespace mjon661 {
	
	namespace impl {
		template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher, 
			 typename Comparer,
			 bool Perfect_Hash>
		class FastHashTableBounce;
		
		template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher,
			 typename Comparer>
		class FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, true>
			: public FastHashTablePerfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>
		{
			using FastHashTablePerfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>::FastHashTablePerfect;
		};
		
		template<typename Elm_t, 
			 typename WrappedBase_t, 
			 typename Key_t, 
			 typename Hasher, 
			 typename Comparer>
		class FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, false>
			: public FastHashTableImperfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>
		{
			using FastHashTableImperfect<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer>::FastHashTableImperfect;
		};
	}
	
	template<typename Elm_t, 
		typename WrappedBase_t, 
		typename Key_t, 
		typename Hasher, 
		typename Comparer,
		bool Perfect_Hash>
	class FastHashTable : 
		public impl::FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, Perfect_Hash>
	{
		using impl::FastHashTableBounce<Elm_t, WrappedBase_t, Key_t, Hasher, Comparer, Perfect_Hash>::FastHashTableBounce;
	};
}
