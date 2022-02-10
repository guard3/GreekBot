#include "Event.h"
#include "Utils.h"

cEvent::cEvent(const json::value& v) : d(v.at("d")) {
	if (int64_t op = v.at("op").as_int64())
		t = (eEvent)op;
	else {
		const char* str = v.at("t").as_string().c_str();
		cUtils::PrintLog("Event: %s", str);
		if (0 == strcmp(str, "READY"))
			t = EVENT_READY;
		else if (0 == strcmp(str, "GUILD_CREATE"))
			t = EVENT_GUILD_CREATE;
		else if (0 == strcmp(str, "GUILD_ROLE_CREATE"))
			t = EVENT_GUILD_ROLE_CREATE;
		else if (0 == strcmp(str, "GUILD_ROLE_UPDATE"))
			t = EVENT_GUILD_ROLE_UPDATE;
		else if (0 == strcmp(str, "GUILD_ROLE_DELETE"))
			t = EVENT_GUILD_ROLE_DELETE;
		else if (0 == strcmp(str, "INTERACTION_CREATE"))
			t = EVENT_INTERACTION_CREATE;
		else if (0 == strcmp(str, "MESSAGE_CREATE"))
			t = EVENT_MESSAGE_CREATE;
		else
			t = EVENT_NOT_IMPLEMENTED;
	}
	const json::value& seq = v.at("s");
	s = seq.is_null() ? 0 : seq.as_int64();
}
