#pragma once

#include <utility>
#include "structs/mem_pool.hpp"


namespace mjon661 {

	template<typename Obj_t>
	class ObjectPool : public MemPool<sizeof(Obj_t), alignof(Obj_t)> {
		
		public:

		template<typename ... Tp>
		Obj_t* construct(Tp&&... args) {
			void* o = this->malloc();
			return reinterpret_cast<Obj_t*>(new (o) Obj_t(std::forward<Tp>(args)...));
		}
		
		void destroy(Obj_t* o) {
			o->~Obj_t();
			this->free(o);
		}
	};

}
