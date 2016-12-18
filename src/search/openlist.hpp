#pragma once

#include <limits>
#include "search/constants.hpp"
#include "structs/fast_binheap.hpp"
#include "util/debug.hpp"


namespace mjon661 { namespace algorithm {
	
	template<typename, typename, typename> class OpenList;
	
	namespace impl {
		static const unsigned Null_Index = std::numeric_limits<unsigned>::max();
		
		template<typename> class OpenListIdxNotifier;
		
		template<typename WrappedBase_t>
		struct OpenListNodeWrapper : public WrappedBase_t {			
			unsigned openind;
			public:
			OpenListNodeWrapper() : openind(Null_Index) {}
		};
		
		template<typename WrappedBase_t>
		struct OpenListIdxNotifier {
			void operator()(OpenListNodeWrapper<WrappedBase_t>* e, unsigned i) const {
				e->openind = i;
			}
		};
	}
	
	
	template<typename Node_t, typename WrappedBase_t, typename Comparer>
	class OpenList : public FastBinheap<
			impl::OpenListNodeWrapper<WrappedBase_t>,
			Comparer,
			impl::OpenListIdxNotifier<WrappedBase_t>> {

		using Base_t = FastBinheap<
			impl::OpenListNodeWrapper<WrappedBase_t>,
			Comparer,
			impl::OpenListIdxNotifier<WrappedBase_t>>;
		
		public:
		
		using Wrapped_t = impl::OpenListNodeWrapper<WrappedBase_t>;
		
		static const unsigned Null_Index = impl::Null_Index;

		OpenList(Comparer pComp) : Base_t(constants::Open_List_Init_Size, pComp) {
		}
		
		void push(Node_t* n_) {
			Wrapped_t* n = static_cast<Wrapped_t*>(n_);
			slow_assert(n->openind == Null_Index);
			Base_t::push(n);
		}

		Node_t* pop() {
			Wrapped_t* n = Base_t::pop();
			slow_assert(n->openind != Null_Index);
			n->openind = Null_Index;
			return n;
		}
		
		void pushOrUpdate(Node_t* n_) {
			Wrapped_t* n = static_cast<Wrapped_t*>(n_);
			if(n->openind == Null_Index)
				push(n);
			else
				Base_t::update(n->openind);
		}
		
		bool contains(Node_t *n) {
			return static_cast<Wrapped_t*>(n)->openind != Null_Index;
		}
		
		void clear() {
			Base_t::clear();
		}
		
		bool empty() {
			return Base_t::empty();
		}
		
		unsigned size() {
			return Base_t::size();
		}
		
		unsigned capacity() {
			return Base_t::capacity();
		}
	};
}}
