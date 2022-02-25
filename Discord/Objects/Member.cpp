#include "Member.h"
#include "json.h"

cMember::cMember(const json::object& o) : joined_at(o.at("joined_at").as_string().c_str()) {
	auto& a = o.at("roles").as_array();
	Roles.reserve(a.size());
	for (auto& v : a)
		Roles.emplace_back(v);
	const json::value* v;
	if ((v = o.if_contains("user")))
		user = cHandle::MakeUnique<cUser>(*v);
	if ((v = o.if_contains("nick"))) {
		if (auto s = v->if_string())
			nick = s->c_str();
	}
	if ((v = o.if_contains("premium_since"))) {
		if (auto s = v->if_string())
			premium_since = s->c_str();
	}
	deaf = (v = o.if_contains("deaf")) && v->as_bool();
	mute = (v = o.if_contains("mute")) && v->as_bool();
}

cMember::cMember(const json::value& v) : cMember(v.as_object()) {}

cMember::cMember(const cMember& o) : nick(o.nick), joined_at(o.joined_at), premium_since(o.premium_since), deaf(o.deaf), mute(o.mute), Roles(o.Roles) {
	if (o.user) user = cHandle::MakeUnique<cUser>(*o.user);
}

cMember& cMember::operator=(const cMember& o) {
	if (o.user) user = cHandle::MakeUnique<cUser>(*o.user);
	nick          = o.nick;
	joined_at     = o.joined_at;
	premium_since = o.premium_since;
	deaf          = o.deaf;
	mute          = o.mute;
	Roles         = o.Roles;
	return *this;
}
