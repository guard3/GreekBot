#include "Interaction.h"

cApplicationCommandInteractionDataOption::cApplicationCommandInteractionDataOption(const json::value& v, const json::value* r) : type(static_cast<eApplicationCommandOptionType>(v.at("type").as_int64())) {
	/* value to object */
	auto& obj = v.as_object();
	/* Initialize name */
	auto& n = v.at("name").as_string();
	name = new char[n.size() + 1];
	strcpy(name, n.c_str());
	/* Initialize value union */
	memset(&value, 0, sizeof(value));
	if (auto c = obj.if_contains("value")) {
		switch (type) {
			case APP_COMMAND_OPT_INTEGER:
				if (auto x = c->if_int64()) value.v_int = static_cast<int>(*x);
				break;
			case APP_COMMAND_OPT_BOOLEAN:
				if (auto x = c->if_bool()) value.v_bll = *x;
				break;
			case APP_COMMAND_OPT_NUMBER:
				if (auto x = c->if_double()) value.v_dbl = *x;
				break;
			default:
				if (auto s = c->if_string()) {
					const char* val = s->c_str();
					try {
						switch (type) {
							case APP_COMMAND_OPT_STRING:
								value.v_str = new char[s->size() + 1];
								strcpy(value.v_str, val);
								break;
							case APP_COMMAND_OPT_USER:
								if (r) {
									value.v_usr.v_usr = new cUser(r->at("users").at(val));
									value.v_usr.v_mbr = new cMember(r->at("members").at(val));
								}
								break;
							default:
								if (r) value.v_sfl = new cSnowflake(val);
								break;
						}
					}
					catch (...) {}
				}
				break;
		}
	}
	else if (auto c = obj.if_contains("options")) {
		//TODO: parse options
	}
}

cApplicationCommandInteractionDataOption::cApplicationCommandInteractionDataOption(const cApplicationCommandInteractionDataOption& o) : name(nullptr), type(o.type) {
	/* Initialize name */
	if (o.name) {
		name = new char[strlen(o.name) + 1];
		strcpy(name, o.name);
	}
	/* Initialize value */
	memcpy(&value, &o.value, sizeof(value));
	switch (type) {
		case APP_COMMAND_OPT_STRING:
			if (o.value.v_str) {
				value.v_str = new char[strlen(o.value.v_str) + 1];
				strcpy(value.v_str, o.value.v_str);
			}
			break;
		case APP_COMMAND_OPT_USER:
			if (o.value.v_usr.v_usr)
				value.v_usr.v_usr = new cUser(*o.value.v_usr.v_usr);
			if (o.value.v_usr.v_mbr)
				value.v_usr.v_mbr = new cMember(*value.v_usr.v_mbr);
			break;
		case APP_COMMAND_OPT_CHANNEL:
		case APP_COMMAND_OPT_ROLE:
			if (o.value.v_sfl)
				value.v_sfl = new cSnowflake(*o.value.v_sfl);
			break;
		default:
			break;
	}
}

cApplicationCommandInteractionDataOption::cApplicationCommandInteractionDataOption(cApplicationCommandInteractionDataOption&& o) noexcept : type(o.type) {
	/* Move name */
	name = o.name;
	o.name = nullptr;
	/* Move value */
	memcpy(&value, &o.value, sizeof(value));
	memset(&o.value, 0, sizeof(o.value));
}

cApplicationCommandInteractionDataOption::~cApplicationCommandInteractionDataOption() {
	delete[] name;
	switch (type) {
		case APP_COMMAND_OPT_STRING:
			delete[] value.v_str;
			break;
		case APP_COMMAND_OPT_USER:
			delete value.v_usr.v_usr;
			delete value.v_usr.v_mbr;
			break;
		case APP_COMMAND_OPT_CHANNEL:
		case APP_COMMAND_OPT_ROLE:
			delete value.v_sfl;
			break;
		default:
			break;
	}
}

cApplicationCommandInteractionDataOption& cApplicationCommandInteractionDataOption::operator=(cApplicationCommandInteractionDataOption o) {
	/* Swap name */
	char* n = name;
	name = o.name;
	o.name = n;
	/* Copy type */
	type = o.type;
	/* Swap value */
	switch (type) {
		case APP_COMMAND_OPT_STRING:
		case APP_COMMAND_OPT_USER:
		case APP_COMMAND_OPT_CHANNEL:
		case APP_COMMAND_OPT_ROLE: {
			char v[sizeof(value)];
			memcpy(v, &value, sizeof(value));
			memcpy(&value, &o.value, sizeof(value));
			memcpy(&o.value, v, sizeof(value));
			break;
		}
		default:
			memcpy(&value, &o.value, sizeof(value));
			break;
	}
	return *this;
}
