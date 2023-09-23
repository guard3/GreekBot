#include "Emoji.h"
#include "json.h"
#include <fmt/format.h>

std::string
cEmoji::ToString() const {
	return fmt::format("<{}:{}:{}>", m_animated ? "a" : "", m_name, m_id);
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmoji& e) {
	v = {
		{ "name",     e.GetName()          },
		{ "animated", e.IsAnimated()       },
		{ "id",       e.GetId().ToString() }
	};
}