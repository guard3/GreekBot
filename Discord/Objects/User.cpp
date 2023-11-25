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

struct crefUser::user_ref {
	const cSnowflake&  id;
	std::string_view hash;
	std::uint16_t   discr;
};

crefUser::crefUser(const cUser& user) noexcept : m_value(&user),
	m_fId([](const void* pVoid) noexcept -> const cSnowflake& {
		return reinterpret_cast<const cUser*>(pVoid)->GetId();
	}),
	m_fAvatar([](const void* pVoid) noexcept {
		return reinterpret_cast<const cUser*>(pVoid)->GetAvatar();
	}),
	m_fDiscriminator([](const void* pVoid) noexcept {
		return reinterpret_cast<const cUser*>(pVoid)->GetDiscriminator();
	}) {}
crefUser::crefUser(const user_ref& user) noexcept : m_value(&user),
	m_fId([](const void* pVoid) noexcept -> const cSnowflake& {
		return reinterpret_cast<const user_ref*>(pVoid)->id;
	}),
	m_fAvatar([](const void* pVoid) noexcept {
		return reinterpret_cast<const user_ref*>(pVoid)->hash;
	}),
	m_fDiscriminator([](const void* pVoid) noexcept {
		return reinterpret_cast<const user_ref*>(pVoid)->discr;
	}) {}
crefUser::crefUser(const cSnowflake &id, std::string_view hash, std::uint16_t discr) noexcept : crefUser(user_ref{ id, hash, discr }) {}