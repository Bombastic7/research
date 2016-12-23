#pragma once


#include "domain/pancake/domain_stack.hpp"


namespace mjon661 { namespace pancake {
	
	template<unsigned N, bool Use_H>
	struct Pancake_DomainStack_single;
	
	template<unsigned N, unsigned Abt1Sz, unsigned AbtStep>
	struct Pancake_DomainStack_IgnoreAbt;

}}
