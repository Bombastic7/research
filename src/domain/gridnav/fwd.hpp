#pragma once

#include "domain/gridnav/flat_stack.hpp"


/** \file domain/gridnav/fwd.hpp
 * gridnav domains
 */
namespace mjon661 { namespace gridnav { namespace flatlayout {
	 

	namespace doms {
		
		
		
		namespace fourway {
			
			//template<unsigned H, unsigned W, bool Use_LC, bool Use_H>
			//struct FourWayBase;
		
			namespace h10w10 {
				
				/** \addtogroup gridnav_DOMAINS 
				*  @{
				*/
				using UnitCost_NoH = GridNav_FlatStack<FourWayBase<10, 10, false, false>>;
				using UnitCost_H = GridNav_FlatStack<FourWayBase<10, 10, false, true>>;
				
				using LifeCost_NoH = GridNav_FlatStack<FourWayBase<10, 10, true, false>>;
				using LifeCost_H = GridNav_FlatStack<FourWayBase<10, 10, true, true>>;
				
				/** @}*/
			}
			
		}
		
		
		namespace eightway {
			
			//template<unsigned H, unsigned W, bool Use_LC, bool Use_H>
			//struct EightWayBase;
			
			namespace h10w10 {
				/** \addtogroup gridnav_DOMAINS 
				*  @{
				*/
				
				using UnitCost_NoH = GridNav_FlatStack<EightWayBase<10, 10, false, false>>;
				using UnitCost_H = GridNav_FlatStack<EightWayBase<10, 10, false, true>>;
				
				using LifeCost_NoH = GridNav_FlatStack<EightWayBase<10, 10, true, false>>;
				using LifeCost_H = GridNav_FlatStack<EightWayBase<10, 10, true, true>>;
				
				/** @}*/
			}
		}
		
		
		
	}

}}}
