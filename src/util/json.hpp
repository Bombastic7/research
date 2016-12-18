#pragma once

#include "util/impl/json.hpp"

namespace mjon661 {
	using Json = nlohmann::json;
	
	/*
	namespace impl {
		void json_check_object(const Json& j, const Json& tmplt) {
			for(auto it = tmplt.cbegin(); it != tmplt.cend(); ++it) {
				auto field_it = j.find(it.key());
				if(field_it == j.cend())
					throw std::runtime_error(std::string("Parse failed. Field \"") + it.key() + "\" was not found.");
				
				if((*field_it).type() != (*it).type())
					throw std::runtime_error(std::string("Parse failed. Field \"") + it.key() + "\" is not of the correct type.");
				 
				if((*field_it).is_object())
					json_check_object(*field_it, *it);
			}
		}
	}
	
	
	//Check that object j has every field in tmplt. Else throw std::runtime_error.
	bool json_check(const Json& j, const Json& tmplt) {
		try {
			if(!j.is_object() || !tmplt.is_object())
				throw std::runtime_error("json_check: arguments must be objects");
			
			impl::json_check_object(j, tmplt);
		} catch(std::runtime_error& e) {
			return false;
		}
		return true;
	}
	*/
}
