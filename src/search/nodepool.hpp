#pragma once

#include <type_traits>
#include "structs/mem_pool.hpp"


namespace mjon661 { namespace algorithm {
	
	/*
	 * A custom pool for Node types typically used. Allocates objects of type
	 *  WrappedNode_t, and refers to them through pointers of type Node_t for 
	 *  convenience.
	 * 
	 * Note that Node_t (as well as  WrappedNode_t) is forced to be default 
	 * constructable and trivially destructible.
	 * 
	 */
	
	template<typename Node_t, typename WrappedNode_t>
	class NodePool : protected MemPool<sizeof(WrappedNode_t), alignof(WrappedNode_t)> {

		static_assert(std::is_trivially_destructible<WrappedNode_t>::value, "Destructors are not called.");
		
		using MemPool_t = MemPool<sizeof(WrappedNode_t), alignof(WrappedNode_t)>;
		
		public:
		
		Node_t* construct() {
			void* n = this->malloc();
			return static_cast<Node_t*>(new (n) WrappedNode_t);
		}
		
		void destroy(Node_t* n) {
			this->free(n);
		}
		
		void clear() {
			MemPool_t::clear();
		}

	};
}}
