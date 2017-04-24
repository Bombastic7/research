
#include <fstream>
#include "experiment/test_all_doms.hpp"

#include "util/json.hpp"
#include "util/debug.hpp"



namespace mjon661 { namespace experiment {

	static void run(int argc, const char* argv[]) {		
		Json jRes;
		select_alg_weight(jRes);
		
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
