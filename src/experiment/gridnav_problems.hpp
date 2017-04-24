#pragma once

#include <utility>
#include <string>
#include <vector>


namespace mjon661 { namespace gridnav { namespace dim2 {


	const unsigned topabtlevel_2_cellmap_1k_0_35 = 6;
	const unsigned topabtlevel_2_cellmap_2k_0_35 = 7;

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


	//init/goal states for 2000*2000 cellmap.
	//Cellmap initialise with setRandom(2000,2000,0,0.35).
	//Problem i generated with DomainStack::genRandInitAndGoal(2000, 5000, i).
	std::pair<unsigned,unsigned> instances_cellmap_2k_0_35(unsigned i) {
		std::vector<std::pair<unsigned,unsigned>> v {
			{975940,2872065}, // (1940,487) -> (65,1436)
			{3464548,379830}, // (548,1732) -> (1830,189)
			{3996576,267928}, // (576,1998) -> (1928,133)
			{2559970,284198}, // (1970,1279) -> (198,142)
			{3936424,539727}, // (424,1968) -> (1727,269)
			{233888,3176381}, // (1888,116) -> (381,1588)
			{2948264,537967}, // (264,1474) -> (1967,268)
			{3573849,1448132}, // (1849,1786) -> (132,724)
			{62129,2719659}, // (129,31) -> (1659,1359)
			{1437931,3618227}, // (1931,718) -> (227,1809)
		};
		return v.at(i);
	}
		
}}}
