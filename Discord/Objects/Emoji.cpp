#include "Emoji.h"
#include "json.h"

json::object cEmoji::ToJson() const {
	json::object obj {
		{ "name",     name     },
		{ "animated", animated }
	};
	if (id)
		obj["id"] = id->ToString();
	return obj;
}