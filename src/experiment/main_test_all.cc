
#include <fstream>
#include "experiment/test_all_doms.hpp"

#include "util/json.hpp"
#include "util/debug.hpp"



namespace mjon661 { namespace experiment {

	static void run(int argc, const char* argv[]) {
		Json jTgt;
		
		if(argc > 1) {
			std::ifstream ifs(argv[1]);
			fast_assert(ifs);
			ifs >> jTgt;
		}
		
		Json jRes;
		select_alg_weight(jRes, jTgt);
		
		{
			std::ofstream out("res.json");
			fast_assert(out);
			out << jRes.dump(2);
		}
	}
}}


int main(int argc, const char* argv[]) {
	mjon661::experiment::run(argc, argv);
}
