#include "Event.h"
#include "Utils.h"
#include <zlib.h>

cEvent::cEvent(json::object& o) : m_name(o.at("t").as_string().c_str()), m_seq(o.at("s").as_int64()), m_data(std::move(o.at("d"))) {
	m_type = (eEvent)crc32(0, (const Bytef*)m_name.c_str(), m_name.length());
}
