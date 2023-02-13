#include "Emoji.h"
#include "json.h"

json::object cEmoji::ToJson() const {
	return {
		{ "name",     m_name          },
		{ "animated", m_animated      },
		{ "id",       m_id.ToString() }
	};
}