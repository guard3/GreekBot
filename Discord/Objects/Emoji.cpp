#include "Emoji.h"
#include "json.h"
#include <fmt/format.h>

std::string cEmoji::ToString() const {
	return fmt::format("<{}:{}:{}>", m_animated ? "a" : "", m_name, m_id);
}

json::object cEmoji::ToJson() const {
	return {
		{ "name",     m_name          },
		{ "animated", m_animated      },
		{ "id",       m_id.ToString() }
	};
}