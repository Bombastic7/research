#pragma once

#include "domain/tiles/defs.hpp"
#include "domain/tiles/domain_basic.hpp"
#include "domain/tiles/common.hpp"


namespace mjon661 { namespace tiles {
	
	namespace dom_8 {
		
		 /** \addtogroup tiles_DOMAINS
		 * @{
		 */
		
		using NoH = TilesAbtDomain_DropAndTaper<3,3,5,true,false,false>;		/**< N=9, unit cost, no heuristic, D&T(5) abt*/
		using MD  = TilesDomain<3,3,true,true,false>;							/**< N=9, unit cost, MD heuristic, no abt */
		using Weighted_NoH = TilesAbtDomain_DropAndTaper<3,3,5,true,false,true>;/**< N=9, Weighted cost, no heuristic, D&T(5) abt */
		
		//* @} */
	}
	
	namespace dom_15 {
		
		/** \addtogroup tiles_DOMAINS
		 * @{
		 */
		 
		using NoH = TilesAbtDomain_DropAndTaper<4,4,8,true,false,false>;		/**< N=16, unit cost, no heuristic, D&T(5) abt*/
		using MD  = TilesDomain<4,4,true,true,false>;							/**< N=16, unit cost, MD heuristic, no abt */
		using Weighted_NoH = TilesAbtDomain_DropAndTaper<4,4,8,true,false,true>;/**< N=16, Weighted cost, no heuristic, D&T(5) abt */
		
		//* @} */
	}
	
	
}}


