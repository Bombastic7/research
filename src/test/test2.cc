
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "util/json.hpp"

#include "domain/tiles/fwd.hpp"


namespace mjon661 {
	
	template<typename = void>
	Json& getJsonLeaf(Json& j, unsigned pos, std::vector<std::string>& pKey) {
		if(pos == pKey.size())
			return j;
		std::string k = pKey[pos];
		return getJsonLeaf(j[k], pos+1, pKey);
	}
	
	
	static void run() {
		Json j;
		
		//std::vector<std::string> v = {"foo", "bar", "baz"};
		std::vector<std::string> v = {};
		Json& jLeaf = getJsonLeaf(j, 0, v);
		
		jLeaf["lorem"] = "ipsum";
		
		std::cout << j.dump(2) << "\n";
	}
}



int main(int argc, const char* argv[]) {

	mjon661::run();
}
