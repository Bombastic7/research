#pragma once

#include <string>

#include "util/debug.hpp"
#include "util/json.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav4 {

	template<typename = void>
	struct NullStatsManager {
		
		template<unsigned L>
		struct StatsAcc {
			
			StatsAcc(NullStatsManager<>& pManager) {}
			void a_expd() { }
			void a_gend() {  }
			void a_dups() {  }
			void a_reopnd() { }
			
			void l_cacheMadeExact() { }
			void l_cacheImprove() { }
			void l_cacheAdd() { }
			
			void s_openListSize(unsigned sz) { }
			void s_closedListSize(unsigned sz) { }
			void s_solutionPartial() {  }
			void s_solutionFull() {  }
			void s_cachePartial() {}
			void s_cacheMiss() {  }
			void s_cacheHit() {  }
			
			void s_end() { }

			void reset() {

			}

			
			void submit() {
			}
		};
		
				
		Json report() {
			return Json();
		}
		
		
		void reset() {
		}
	};
}}}
