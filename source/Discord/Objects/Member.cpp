#include "Member.h"

cMember::cMember(const json::value& v) : user([](const json::value& v) {
	try {
		return std::make_unique<const cUser>(v.at("user"));
	}
	catch (const std::exception&) {
		return uchUser();
	}
} (v)), nick([](const json::value& v) {
	try {
		return std::make_unique<const std::string>(v.at("nick").as_string().c_str());
	}
	catch (const std::exception&) {
		return uchHandle<std::string>();
	}
} (v)) {
	const json::array& a = v.at("roles").as_array();
	auto& r = const_cast<std::vector<chSnowflake>&>(Roles);
	roles.reserve(a.size());
	r.reserve(a.size());
	for (auto& val : a) {
		roles.push_back(val.as_string().c_str());
		r.push_back(&roles.back());
	}
}
