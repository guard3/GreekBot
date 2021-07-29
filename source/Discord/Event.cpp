#include "Event.h"
#include "Utils.h"

cEvent::cEvent(const json::value& v) : d(v.at("d")) {
	auto op = v.at("op").as_int64();
	if (op) {
		t = static_cast<eEvent>(op);
		cUtils::PrintLog("Opcode: %d", t);
	}
	else {
		const char* str = v.at("t").as_string().c_str();
		cUtils::PrintLog("Event:  %s", str);
		if (0 == strcmp(str, "READY"))
			t = EVENT_READY;
		else if (0 == strcmp(str, "INTERACTION_CREATE"))
			t = EVENT_INTERACTION_CREATE;
		else
			t = EVENT_NOT_IMPLEMENTED;
	}
	const json::value& seq = v.at("s");
	s = seq.is_null() ? 0 : static_cast<int>(seq.as_int64());
}
