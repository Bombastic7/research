#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "domain/gridnav/defs.hpp"
#include "domain/gridnav/flat_domain.hpp"

#include "util/json.hpp"
#include "util/exception.hpp"




namespace mjon661 { namespace gridnav { namespace blocked {


	template<unsigned Height, unsigned Width, template<unsigned, unsigned> typename DB>
	struct GridNav_Stack_single {
		
		using selfStack_t = GridNav_Stack_single<Height, Width, DB>;
		
		
		template<unsigned L>
		struct Domain : GridNav_Dom<Height, Width, DB> {
			
			static_assert(L == 0, "");
			
			Domain(selfStack_t& pStack) :
				GridNav_Dom<Height, Width, DB>(
			
			
		};
		
		
		
		
		
		
	};



	template<typename DomBase>
	struct GridNav_FlatStack {
		
		static const int Height = DomBase::Height, Width = DomBase::Width;
		
		
		template<unsigned L>
		struct Domain : public GridNav_Dom<DomBase> {
			
			static_assert(L == 0, "");
			
			using base_t = GridNav_Dom<DomBase>;
			
			
			Domain(GridNav_FlatStack<DomBase>& pStack) :
				base_t(*pStack.mCells, pStack.mInitPos, pStack.mGoalPos)
			{}
		};
		
		
		
		GridNav_FlatStack(Json const& jConfig) :
			mInitPos(readInitGoalCoord(jConfig, "init")),
			mGoalPos(readInitGoalCoord(jConfig, "goal"))
		{
			mCells = new CellArray<Height,Width>;
			
			std::ifstream ifs(jConfig.at("map").get<std::string>());
			
			if(!ifs)
				throw ConfigException("Could not open map file");
			
			mCells->read(ifs);
		}
		
		~GridNav_FlatStack() {
			delete mCells;
		}
		
		
		idx_t readInitGoalCoord(Json const& jConfig, std::string const& key) {
			if(jConfig.at(key).is_number())
				return jConfig.at(key).get<idx_t>();
			
			else if(jConfig.at(key).is_array()) {
				idx_t x = jConfig.at(key).at(0);
				idx_t y = jConfig.at(key).at(1);
				
				if(x < 0 || x >= Width || y < 0 || y >= Height)
					throw ConfigException("Coords out of range");
				
				return y * Width + x;
			} else
				throw ConfigException("Coords have bad type");
			
			return 0;
		}
		
		
		const idx_t mInitPos, mGoalPos;
		
		CellArray<Height, Width>* mCells;
		
	};

}}}
