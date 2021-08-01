#include "Interaction.h"

cInteractionDataOption::cInteractionDataOption(const json::value& v) : name(v.at("name").as_string().c_str()), type(static_cast<eApplicationCommandType>(v.at("type").as_int64())), value(v.at("value")) {
	if (type == APP_COMMAND_USER || type == APP_COMMAND_CHANNEL || type == APP_COMMAND_ROLE) {
		if (auto p = value.if_string())
			m_snowflake.reset(new cSnowflake(p->c_str()));
	}
}

cInteractionData::cInteractionData(const json::value& v) : id(v.at("id").as_string().c_str()), name(v.at("name").as_string().c_str()), resolved([](const json::value& v) {
	try {
		return cInteractionDataResolved(v.at("resolved"));
	}
	catch (const std::exception&) {
		return cInteractionDataResolved();
	}
} (v)) {
	auto& opt = const_cast<std::vector<chInteractionDataOption>&>(Options);
	try {
		const json::array& a = v.at("options").as_array();
		options.reserve(a.size());
		opt.reserve(a.size());
		for (const json::value& val : a) {
			options.push_back(cInteractionDataOption(val));
			opt.push_back(&options.back());
		}
	}
	catch (const std::exception&) {
		options.clear();
		opt.clear();
	}
}
