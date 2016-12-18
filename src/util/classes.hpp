#pragma once


namespace mjon661 {
	
	template<typename R = void>
	struct NullFunctor {
	
		template<typename... Tp>
		NullFunctor(Tp...) {}
		
		template<typename... Tp>
		R operator()(Tp...) {
			return R();
		}
	
	};
	
}
