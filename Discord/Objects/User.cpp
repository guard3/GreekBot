#include "User.h"
#include "Utils.h"
#include "json.h"

static std::string as_string(const json::value& v) {
	return v.is_null() ? std::string() : json::value_to<std::string>(v);
}

cUser::cUser(const json::value& v) : cUser(v.as_object()) {}
cUser::cUser(const json::object& o):
	m_id(o.at("id").as_string()),
	m_username(json::value_to<std::string>(o.at("username"))),
	m_avatar(as_string(o.at("avatar"))),
	m_global_name(as_string(o.at("global_name"))) {
	/* Combine username and discriminator to account for new usernames */
	auto disc = json::value_to<std::string_view>(o.at("discriminator"));
	m_discriminator = cUtils::ParseInt<uint16_t>(disc);
	if (m_discriminator)
		m_username = fmt::format("{}#{}", m_username, disc);

	const json::value* p;
	p = o.if_contains("bot");
	m_bot = p && p->as_bool();
	p = o.if_contains("system");
	m_system = p && p->as_bool();
}