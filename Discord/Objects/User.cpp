#include "User.h"
#include "Utils.h"
#include <boost/json.hpp>

namespace json = boost::json;

cUser::cUser(const json::value& v) : cUser(v.as_object()) {}
cUser::cUser(const json::object& o):
	m_id(o.at("id").as_string()),
	m_avatar([&o] {
		auto& s = o.at("avatar");
		return s.is_null() ? std::string() : json::value_to<std::string>(s);
	}()),
	m_global_name([&o] {
		auto& s = o.at("global_name");
		return s.is_null() ? std::string() : json::value_to<std::string>(s);
	}()) {
	/* Combine username and discriminator to account for new usernames */
	std::string_view disc_str = o.at("discriminator").as_string();
	std::string_view username = o.at("username").as_string();
	auto disc_int = cUtils::ParseInt<std::uint16_t>(disc_str);
	m_username = disc_int ? fmt::format("{}#{}", username, disc_str) : (std::string)username;
	m_discriminator = disc_int;

	const json::value* p;
	m_bot = (p = o.if_contains("bot")) && p->as_bool();
	m_system = (p = o.if_contains("system")) && p->as_bool();
}