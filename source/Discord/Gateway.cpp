#include "Gateway.h"
#include "Discord.h"

cSessionStartLimit::cSessionStartLimit(const json::value& v) {
	try {
		/* Get session_start_limit object from JSON */
		const json::object& obj = v.as_object().at("session_start_limit").as_object();
		
		/* Parse JSON object */
		m_total           = static_cast<int>(obj.at("total"          ).as_int64());
		m_remaining       = static_cast<int>(obj.at("remaining"      ).as_int64());
		m_reset_after     = static_cast<int>(obj.at("reset_after"    ).as_int64());
		m_max_concurrency = static_cast<int>(obj.at("max_concurrency").as_int64());
	}
	catch (const std::exception&) {
		/* On error, set everything to 0 */
		m_total = m_remaining = m_reset_after = m_max_concurrency = 0;
	}
}

cGateway::cGateway(const char* auth) {
	/* Retrieve gateway object */
	if (cDiscord::GetGateway(auth, m_json)) {
		try {
			m_url = m_json.as_object().at("url").as_string().c_str();
			m_shards = static_cast<int>(m_json.as_object().at("shards").as_int64());
			return;
		}
		catch (const std::exception&) {}
	}
	m_errorObject = new cJsonError(m_json);
	m_session_start_limit = new cSessionStartLimit(m_json);
}

cGateway::~cGateway() {
	delete m_errorObject;
	delete m_session_start_limit;
}
