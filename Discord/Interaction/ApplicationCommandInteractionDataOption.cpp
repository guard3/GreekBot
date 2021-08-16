#include "Interaction.h"

cApplicationCommandInteractionDataOption::cApplicationCommandInteractionDataOption(const json::value& v, const json::value* r) : name(v.at("name").as_string().c_str()), type(static_cast<eApplicationCommandOptionType>(v.at("type").as_int64())) {
	memset(&value, 0, sizeof(value));
	switch (type) {
		case APP_COMMAND_OPT_SUB_COMMAND:
		case APP_COMMAND_OPT_SUB_COMMAND_GROUP:
			// TODO: parse options
			break;
		case APP_COMMAND_OPT_STRING: {
			auto& s = v.at("value").as_string();
			value.v_str = new char[s.size() + 1];
			strcpy(value.v_str, s.c_str());
			break;
		}
		case APP_COMMAND_OPT_INTEGER:
			value.v_int = static_cast<int>(v.at("value").as_int64());
			break;
		case APP_COMMAND_OPT_BOOLEAN:
			value.v_bll = v.at("value").as_bool();
			break;
		case APP_COMMAND_OPT_NUMBER:
			value.v_dbl = v.at("value").as_double();
			break;
		case APP_COMMAND_OPT_USER:
			if (r) {
				try {
					auto s = v.at("value").as_string().c_str();
					value.v_usr.v_usr = new cUser(r->at("users").at(s));
					value.v_usr.v_mbr = new cMember(r->at("members").at(s));
				}
				catch (...) {}
			}
			break;
		default:
			value.v_sfl = new cSnowflake(v.at("value"));
			break;
	}
}

cApplicationCommandInteractionDataOption::cApplicationCommandInteractionDataOption(const cApplicationCommandInteractionDataOption& o) : name(o.name), type(o.type) {
	memcpy(&value, &o.value, sizeof(value));
	switch (type) {
		case APP_COMMAND_OPT_STRING:
			if (o.value.v_str) {
				value.v_str = new char[strlen(o.value.v_str) + 1];
				strcpy(value.v_str, o.value.v_str);
			}
			break;
		case APP_COMMAND_OPT_USER:
			try {
				if (o.value.v_usr.v_usr)
					value.v_usr.v_usr = new cUser(*o.value.v_usr.v_usr);
				if (o.value.v_usr.v_mbr)
					value.v_usr.v_mbr = new cMember(*value.v_usr.v_mbr);
			}
			catch (const std::exception& e) {
				delete value.v_usr.v_usr;
				throw e;
			}
			catch (...) {
				delete value.v_usr.v_usr;
			}
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

cApplicationCommandInteractionDataOption::cApplicationCommandInteractionDataOption(cApplicationCommandInteractionDataOption&& o) noexcept : name(std::move(o.name)), type(o.type) {
	memcpy(&value, &o.value, sizeof(value));
	memset(&o.value, 0, sizeof(o.value));
}

cApplicationCommandInteractionDataOption::~cApplicationCommandInteractionDataOption() {
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
	name.swap(o.name);
	type = o.type;
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