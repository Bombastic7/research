#pragma once


#include "domain/pancake/domain_stack.hpp"
#include "domain/pancake/domain.hpp"

/** \file domain/pancake/fwd.hpp
 * Declarations for Pancake concept.
 */

namespace mjon661 { namespace pancake {
	
	
	 
	 namespace doms10 {
		 
		 
		 
		 namespace noheuristic {
			 /** \addtogroup pancake_DOMAINS
			 * @{
			 */
			 

			/** N=10, unit cost, no h, DropAndTaper abt.*/
			using Abt5 = DomainStack<10, 5, Domain_NoH>;
			
			/** @}*/
		 }
		 
		 namespace gapheuristic {
			 
			 /** \addtogroup pancake_DOMAINS
			 * @{
			 */
			 
			/**
			 * N=10, unit cost, gap heuristic, no abs.
			 */
			using Abt5 = DomainStack<10, 5, Domain_GapH>;
			
			/** @}*/
			
		 }
		 
		 
	 }
	
	

}}
