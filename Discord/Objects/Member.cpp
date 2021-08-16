#include "Member.h"

cMember::cMember(const json::value& v) {
	/* Initialize user */
	uhUser u_user;
	if (auto c = v.as_object().if_contains("user"))
		u_user = cHandle::MakeUniqueNoEx<cUser>(*c);
	/* Initialize nick */
	uhHandle<char[]> u_nick;
	try {
		auto& s = v.at("nick").as_string();
		u_nick = cHandle::MakeUnique<char[]>(s.size() + 1);
		strcpy(u_nick.get(), s.c_str());
	}
	catch (...) {}
	/* Initialize roles */
	auto& a = v.at("roles").as_array();
	roles.reserve(a.size());
	Roles.reserve(a.size());
	for (auto& e : a) {
		roles.emplace_back(e);
		Roles.push_back(&roles.back());
	}
	/* Copy pointers */
	user = u_user.release();
	nick = u_nick.release();
}

cMember::cMember(const cMember& o) : roles(o.roles) {
	/* Initialize user */
	uhUser u_user = o.user ? cHandle::MakeUnique<cUser>(*o.user) : uhUser();
	/* Initialize nickname */
	uhHandle<char[]> u_nick;
	if (o.nick) {
		u_nick = cHandle::MakeUnique<char[]>(strlen(o.nick) + 1);
		strcpy(u_nick.get(), o.nick);
	}
	/* Initialize roles */
	Roles.reserve(roles.size());
	for (auto& e : roles)
		Roles.push_back(&e);
	/* Copy pointers */
	user = u_user.release();
	nick = u_nick.release();
}

cMember::cMember(cMember &&o) noexcept : roles(std::move(o.roles)), Roles(std::move(o.Roles)) {
	user = o.user;
	nick = o.nick;
	o.user = nullptr;
	o.nick = nullptr;
}

cMember::~cMember() {
	delete   user;
	delete[] nick;
}

cMember& cMember::operator=(cMember o) {
	std::swap(user, o.user);
	std::swap(nick, o.nick);
	roles.swap(o.roles);
	Roles.swap(o.Roles);
	return *this;
}
