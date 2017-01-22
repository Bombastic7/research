#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "util/debug.hpp"
#include "util/json.hpp"



namespace mjon661 { namespace algorithm { namespace ugsav5 {
	

	template<typename = void>
	struct SimpleStatsManager {
		
		template<unsigned L>
		struct StatsAcc {
			
			
			struct S_Record {
				unsigned expd, gend, dups, reopnd;
				bool cachedsol;
				unsigned nsearch;
				double hbf;
				void reset() { expd = gend = dups = reopnd = 0; cachedsol = false, nsearch = -1; hbf = 0;}
				
				static void writeNames(std::ostream& out) {
					out << "n expd gend dups reopnd hbf cachedsol\n";
				}
				
				void writeRow(std::ostream& out) {
					out << nsearch << " " << expd << " " << gend << " " << dups << " " << reopnd << " " << hbf << " " << (int)cachedsol << "\n";
				}
			};
			
			StatsAcc(SimpleStatsManager<>& pManager) :
				mManager(pManager)
			{
				reset();
			}
			
			
			void a_expd() { mSrecord.expd++; }
			void a_gend() { mSrecord.gend++; }
			void a_dups() { mSrecord.dups++; }
			void a_reopnd() { mSrecord.reopnd++; }

			void l_cacheAdd() { mLcachedStates++;  }
			void l_cacheImprove() {}
			void l_cacheMadeExact() {}
			
			void s_cacheMiss() {}
			void s_cachePartial() {}
			void s_cacheHit() {}
			
			void s_solutionFull() {}
			void s_solutionPartial() {}


			void s_openListSize(unsigned) {}
			void s_closedListSize(unsigned) {}


			void s_end() { 
				mSrecord.nsearch = mLnsearches++;
				mLtotalExpd += mSrecord.expd;
				mLtotalGend += mSrecord.gend;
				
				if(L == 0) mLallSearches.push_back(mSrecord);
				mSrecord.reset();
			}
			
			unsigned getExpd() {
				return mSrecord.expd;
			}

			
			S_Record mSrecord;
			unsigned mLnsearches, mLtotalExpd, mLtotalGend, mLcachedStates;
			std::vector<S_Record> mLallSearches;
			bool mWasCachedSolution;



			
			void reset() {
				mLnsearches = mLtotalExpd = mLtotalGend = mLcachedStates = 0;
				mSrecord.reset();
				mLallSearches.clear();
			}

			
			void submit(Json j = Json(), int branch = -1) {
				
				if(L == 0) {
					gen_assert(branch == -1);
					
					S_Record l0search = mLallSearches.at(0);
					j["expd"] = l0search.expd;
					j["gend"] = l0search.gend;
					j["dups"] = l0search.dups;
					j["reopnd"] = l0search.reopnd;
				}
				
				if(L > 0) {
					j["NSearches"] = mLnsearches;
					j["Cached states"] = mLcachedStates;
					
					std::stringstream ss;
					//S_Record::writeNames(ss);
					
					for(unsigned i=0; i<mLallSearches.size(); i++) {
						mLallSearches[i].writeRow(ss);
					}
					
					//j["search log"] = ss.str();
				}

				std::string lvlkey = std::string("Level ") + std::to_string(L);
		
				if(branch >= 0)
					mManager.mReport[lvlkey][std::string("branch ") + std::to_string(branch)] = j;
				else
					mManager.mReport[lvlkey] = j;
					
				mManager.mTexpd += mLtotalExpd;
				mManager.mTgend += mLtotalGend;
			}
			
			private:
			SimpleStatsManager<>& mManager;

		};

		
		
		SimpleStatsManager() {
			reset();
		}
		
		Json report() {
			mReport["_all_expd"] = mTexpd;
			mReport["_all_gend"] = mTgend;
			
			mReport["_base_expd"] = mReport.at("Level 0").at("expd");
			mReport["_base_gend"] = mReport.at("Level 0").at("gend");
			return mReport;
		}
		
		
		void reset() {
			mTexpd = mTgend = 0;
			mReport = Json();
		}
		
		unsigned mTexpd, mTgend;
		Json mReport;
	};
}}}
