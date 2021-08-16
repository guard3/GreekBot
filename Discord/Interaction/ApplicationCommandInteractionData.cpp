#include "Interaction.h"

cApplicationCommandInteractionData::cApplicationCommandInteractionData(const json::value& v) : id(v.at("id")), name(v.at("name").as_string().c_str()), type(static_cast<eApplicationCommandType>(v.at("type").as_int64())) {
	/* Interpret json value as object */
	auto& o = v.as_object();
	/* Initialize target_id */
	uhSnowflake u_target_id;
	if (auto c = o.if_contains("target_id"))
		u_target_id = cHandle::MakeUniqueNoEx<cSnowflake>(*c);
	/* Initialize options */
	if (auto c = o.if_contains("options")) {
		auto& a = c->as_array();
		auto  r = o.if_contains("resolved");
		options.reserve(a.size());
		Options.reserve(a.size());
		for (auto& e : a) {
			options.emplace_back(e, r);
			Options.push_back(&options.back());
		}
	}
	/* Copy pointers */
	target_id = u_target_id.release();
}

cApplicationCommandInteractionData::cApplicationCommandInteractionData(const cApplicationCommandInteractionData &o) : id(o.id), name(o.name), type(o.type), options(o.options) {
	/* Initialize target_id */
	uhSnowflake u_target_id = o.target_id ? cHandle::MakeUnique<cSnowflake>(*o.target_id) : uhSnowflake();
	/* Initialize options */
	Options.reserve(options.size());
	for (auto e : o.options)
		Options.push_back(&e);
	/* Copy pointers */
	target_id = u_target_id.release();
}

cApplicationCommandInteractionData::cApplicationCommandInteractionData(cApplicationCommandInteractionData &&o) noexcept : id(o.id), name(std::move(o.name)), type(o.type), options(std::move(o.options)), Options(std::move(o.Options)) {
	target_id   = o.target_id;
	o.target_id = nullptr;
}

cApplicationCommandInteractionData::~cApplicationCommandInteractionData() {
	delete target_id;
}

cApplicationCommandInteractionData &cApplicationCommandInteractionData::operator=(cApplicationCommandInteractionData o) {
	id = o.id;
	name.swap(o.name);
	type = o.type;
	std::swap(target_id, o.target_id);
	options.swap(o.options);
	Options.swap(o.Options);
}
