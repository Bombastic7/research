#pragma once

#include "domain/gridnav/blocked/domain_stack.hpp"


namespace mjon661 { namespace gridnav { namespace blocked {

	
	
	template<	unsigned Height,
				unsigned Width, 
				bool Use_EightWay, 
				bool Use_LifeCost,
				bool Use_H,
				unsigned Max_Abt_Lvls, 
				unsigned Merge_Height_Factor,
				unsigned Merge_Width_Factor,
				unsigned Merge_Fill_Factor
			>

	struct GridNav_DomainStack_MergeAbt;
	
}}}
