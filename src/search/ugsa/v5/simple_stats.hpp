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
				void reset() { expd = gend = dups = reopnd = 0; cachesol = false, nsearch = -1;}
				
				static void writeNames(std::ostream& out) {
					out << "n expd gend dups reopnd cachedsol\n";
				}
				
				void writeRow(std::ostream& out) {
					out << nsearch << " " << expd << " " << gend << " " << dups << " " << reopnd << " " << (int)cachedsol << "\n";
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
			
			void s_cachedsol() { mSrecord.cachedsol = true; }
			
			void s_end() { 
				mSrecord.nsearch = mLnsearches++;
				mLtotalExpd += mSrecord.expd;
				mLtotalGend += mSrecord.gend;
				
				mLallSearches.push_back(mSrecord);
				mSrecord.reset();
			}

			
			S_Record mSrecord;
			unsigned mLnsearches, mLtotalExpd, mLtotalGend, mLcachedStates;
			std::vector<S_Record> mLallSearches;
			



			
			void reset() {
				mLnsearches = mLtotalExpd = mLtotalGend = mLcachedStates = 0;
				mSrecord.reset();
				mLallSearches.clear();
			}

			
			void submit(Json j = Json()) {
				
				if(L == 0) {
					S_Record l0search = mLallSearches.at(0);
					j["expd"] = l0search.expd;
					j["gend"] = l0search.gend;
					j["dups"] = l0search.dups;
					j["reopnd"] = l0search.reopnd;
				}
				
				if(L > 1) {
					j["NSearches"] = mLnsearches;
					j"Cached stats"] = mLcachedStates;
					
					std::string dumpfile = std::string("ugsa_stats_dump_" +  std::to_string(L));
					std::ofstream ofs(dumpfile);
					
					if(!ofs)
						logDebug(std::string("Cound not open " + dumpfile);
					else {
						j["stats outfile"] = dumpfile;
						S_Record::writeNames(ofs);
						for(unsigned i=0; i<mLallSearches.size(); i++) {
							mLallSearches.writeRow(ofs);
						}
					}
				}

				mManager.mReport[std::string("Level ") + std::to_string(L)] = j;
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
