#include "Event.h"

cEvent::cEvent(const json::value& v) : d(v.at("d")) {
	auto op = v.at("op").as_int64();
	printf("OPCODE: %d\n", (int)op);
	if (op)
		t = static_cast<eEvent>(op);
	else {
		const char* str = v.at("t").as_string().c_str();
		if (0 == strcmp(str, "READY"))
			t = EVENT_READY;
		else
			t = EVENT_NOT_IMPLEMENTED;
	}
	const json::value& seq = v.at("s");
	s = seq.is_null() ? 0 : static_cast<int>(seq.as_int64());
}
