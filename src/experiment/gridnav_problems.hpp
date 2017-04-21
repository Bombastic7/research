#pragma once

#include <utility>
#include <string>
#include <vector>


namespace mjon661 { namespace gridnav { namespace dim2 {


	const unsigned topabtlevel_2_cellmap_1k_0_35 = 6;

	//init/goal states for 1000*1000 cellmap.
	//Cellmap initialise with setRandom(1000,1000,0,0.35).
	//Problem i generated with DomainStack::genRandInitAndGoal(1000, 5000, i).
	std::pair<unsigned,unsigned> instances_cellmap_1k_0_35(unsigned i) {
		std::vector<std::pair<unsigned,unsigned>> v {
			{25213,977870},
			{940882,354013},
			{205064,736931},
			{971945,115281},
			{34917,778087},
			{908026,12578},
			{310056,805942},
			{903535,37019},
			{837065,147790},
			{947955,103162}
		};
		return v.at(i);
	}


}}}
