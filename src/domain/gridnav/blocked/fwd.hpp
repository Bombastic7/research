#pragma once

#include "domain/gridnav/blocked/domain_stack.hpp"



namespace mjon661 { namespace gridnav { namespace blocked {


	template<unsigned H, unsigned W, bool Use_8, bool Use_LC, bool Use_H>
	using GridNav_single_h_w_mv_cst_hr = GridNav_DomainStack_single<H, W, GridNavBase<Use_8, Use_LC, Use_H>>;



	/*
	template<	unsigned Height,
				unsigned Width, 
				bool Use_EightWay, 
				bool Use_LifeCost,
				unsigned Max_Abt_Lvls, 
				unsigned Merge_Height_Factor,
				unsigned Merge_Width_Factor,
				unsigned Merge_Fill_Factor
			>
		= GridNav_DomainStack_MergeAbt
	*/

	template


}}}
