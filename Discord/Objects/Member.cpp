#include "Member.h"

cMember::cMember(const json::value& v) : user(nullptr), nick(nullptr) {
	if (auto o = v.if_object()) {
		/* Initialize user */
		if (auto c = o->if_contains("user"))
			user = new cUser(*c);
		/* Initialize nickname */
		if (auto c = o->if_contains("nick")) {
			if (auto s = c->if_string()) {
				nick = new char[s->size() + 1];
				strcpy(nick, s->c_str());
			}
		}
		/* Initialize roles */
		if (auto c = o->if_contains("roles")) {
			if (auto a = c->if_array()) {
				auto& roles = const_cast<std::vector<chSnowflake>&>(Roles);
				roles.reserve(a->size());
				try {
					for (auto& e : *a)
						roles.push_back(new cSnowflake(e));
				}
				catch (...) {
					roles.clear();
				}
			}
		}
	}
}

cMember::cMember(const cMember& o) : user(nullptr), nick(nullptr) {
	/* Copy user */
	if (o.user)
		user = new cUser(*o.user);
	/* Copy nickname */
	if (o.nick) {
		nick = new char[strlen(o.nick) + 1];
		strcpy(nick, o.nick);
	}
	/* Copy roles */
	if (!o.Roles.empty()) {
		auto& roles = const_cast<std::vector<chSnowflake>&>(Roles);
		roles.reserve(o.Roles.size());
		for (auto p : o.Roles)
			roles.push_back(new cSnowflake(*p));
	}
}

cMember::cMember(cMember &&o) noexcept : Roles(std::move(const_cast<std::vector<chSnowflake>&>(o.Roles))) {
	user = o.user;
	nick = o.nick;
	o.user = nullptr;
	o.nick = nullptr;
}

cMember::~cMember() {
	delete user;
	delete[] nick;
	for (auto p : Roles)
		delete p;
}