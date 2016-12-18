#!/bin/python





#( class name, confType, hasHeuristic, hasAbstraction )
DOMS = [
			("mjon661::tiles::dom_8::NoH", "tiles8",				False, True),
			("mjon661::tiles::dom_8::MD", 	"tiles8",				True, True),
			("mjon661::tiles::dom_8::Weighted_NoH", "tiles8",		False, True),

			("mjon661::pancake::doms10::noheuristic::Abt5", "pancake10", False, True),
			("mjon661::pancake::doms10::gapheuristic::Abt5", "pancake10", True, True),

			("mjon661::gridnav::flatlayout::doms::fourway::h10w10::UnitCost_NoH", "gn10_10", False, False),
			("mjon661::gridnav::flatlayout::doms::fourway::h10w10::UnitCost_H", "gn10_10", True, False),
			("mjon661::gridnav::flatlayout::doms::fourway::h10w10::LifeCost_NoH", "gn10_10", False, False),
			("mjon661::gridnav::flatlayout::doms::fourway::h10w10::LifeCost_H", "gn10_10", True, False),
			
			("mjon661::gridnav::flatlayout::doms::eightway::h10w10::UnitCost_NoH", "gn10_10", False, False),
			("mjon661::gridnav::flatlayout::doms::eightway::h10w10::UnitCost_H", "gn10_10", True, False),
			("mjon661::gridnav::flatlayout::doms::eightway::h10w10::LifeCost_NoH", "gn10_10", False, False),
			("mjon661::gridnav::flatlayout::doms::eightway::h10w10::LifeCost_H", "gn10_10", True, False),
		]
	

#( class name, needsHeuristic, needsAbstraction )
ALGS = [
		("mjon661::algorithm::Astar", False, False),
		("mjon661::algorithm::IDAstar", False, False),
		("mjon661::algorithm::Bugsy", True, False),
		
		("mjon661::algorithm::hastargeneric::HAstar_StatsLevel", False, True),
		("mjon661::algorithm::ugsav1::UGSAv1", False, True),
		("mjon661::algorithm::ugsav2::UGSAv2_StatsLevel", False, True)

		]
