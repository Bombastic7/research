#pragma once

#include "domain/gridnav/blocked/graph.hpp"
#include "domain/gridnav/blocked/domain_stack.hpp"


namespace mjon661 { namespace gridnav { namespace blocked {

	
	/*
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
	*/
	
	template<unsigned Height, unsigned Width, bool Use_8way, bool Use_LifeCost, bool Use_H>
	struct GridNav_DomainStack_StarAbt;

	template<unsigned Height, unsigned Width, bool Use_LifeCost, bool Use_H>
	struct GridNav_DomainStack_StarAbt<Height, Width, false, Use_LifeCost, Use_H> : 
		public GridNav_DomainStack_StarAbt_<Height, Width, FourWayFuncs<>, Use_LifeCost, Use_H> {
		
		using GridNav_DomainStack_StarAbt_<Height, Width, FourWayFuncs<>, Use_LifeCost, Use_H>::GridNav_DomainStack_StarAbt_;
	};

	template<unsigned Height, unsigned Width, bool Use_LifeCost, bool Use_H>
	struct GridNav_DomainStack_StarAbt<Height, Width, true, Use_LifeCost, Use_H> : 
		public GridNav_DomainStack_StarAbt_<Height, Width, EightWayFuncs<>, Use_LifeCost, Use_H> {
		
		using GridNav_DomainStack_StarAbt_<Height, Width, FourWayFuncs<>, Use_LifeCost, Use_H>::GridNav_DomainStack_StarAbt_;
	};

}}}
