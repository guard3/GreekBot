#include "Interaction.h"

cApplicationCommandInteractionData::cApplicationCommandInteractionData(const json::value& v) : id(v.at("id")), type(static_cast<eApplicationCommandType>(v.at("type").as_int64())) {
	/* value as object */
	auto& obj = v.as_object();
	/* Initialize name */
	const json::string& str = v.at("name").as_string();
	name = new char[str.size() + 1];
	strcpy(name, str.c_str());
	/* Initialize target_id */
	if (auto c = obj.if_contains("target_id")) {
		if (auto s = c->if_string())
			target_id = new cSnowflake(s->c_str());
	}
	/* Initialize options */
	if (auto c = obj.if_contains("options")) {
		if (auto a = c->if_array()) {
			auto r = obj.if_contains("resolved");
			auto& options = const_cast<std::vector<chApplicationCommandInteractionDataOption>&>(Options);
			options.reserve(a->size());
			for (auto& e : *a)
				options.push_back(new cApplicationCommandInteractionDataOption(e, r));
		}
	}
}

cApplicationCommandInteractionData::cApplicationCommandInteractionData(const cApplicationCommandInteractionData &o) : id(o.id), type(o.type) {
	/* Copy name */
	if (o.name) {
		name = new char[strlen(o.name) + 1];
		strcpy(name, o.name);
	}
	/* Copy target_id */
	if (o.target_id)
		target_id = new cSnowflake(*o.target_id);
	/* Copy options */
	auto& options = const_cast<std::vector<chApplicationCommandInteractionDataOption>&>(Options);
	options.reserve(o.Options.size());
	for (auto p : o.Options)
		options.push_back(new cApplicationCommandInteractionDataOption(*p));
}

cApplicationCommandInteractionData::cApplicationCommandInteractionData(cApplicationCommandInteractionData &&o) noexcept : id(o.id), type(o.type), Options(std::move(const_cast<std::vector<chApplicationCommandInteractionDataOption>&>(o.Options))) {
	name        = o.name;
	target_id   = o.target_id;
	o.name      = nullptr;
	o.target_id = nullptr;
}

cApplicationCommandInteractionData::~cApplicationCommandInteractionData() {
	delete[] name;
	delete target_id;
}
