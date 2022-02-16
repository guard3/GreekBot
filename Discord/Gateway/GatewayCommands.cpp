#include "Gateway.h"

void
cGateway::resume() {
	send(cUtils::Format(R"({"op":6,"d":{"token":"%s","session_id":"%s","seq":%)" PRIi64 "}}", GetToken(), m_session_id.c_str(), m_last_sequence.load()));
}

void
cGateway::identify() {
	send(cUtils::Format(R"({"op":2,"d":{"token":"%s","intents":%d,"properties":{"$os":"%s","$browser":"GreekBot","$device":"GreekBot"}}})", GetToken(), m_intents, cUtils::GetOS()));
}

void
cGateway::heartbeat() {
	m_heartbeat_ack.store(false);
	if (int64_t s = m_last_sequence)
		send(cUtils::Format(R"({"op":1,"d":%)" PRId64 "}", s));
	else
		send(R"({"op":1,"d":null})");
}

void
cGateway::start_heartbeating(int64_t interval) {
	m_heartbeat_exit.store(false);
	m_heartbeat_thread = std::thread([this](int64_t interval_int) {
		auto interval = std::chrono::milliseconds(interval_int);
		/* Wait for a random amount of time */
		auto later = std::chrono::system_clock::now() + std::chrono::milliseconds(cUtils::Random(0, interval_int));
		while (std::chrono::system_clock::now() < later) {
			if (m_heartbeat_exit.load())
				return;
		}
		/* Begin the heartbeating loop */
		do {
			heartbeat();
			later = std::chrono::system_clock::now() + interval;
			while (std::chrono::system_clock::now() < later) {
				if (m_heartbeat_exit.load())
					return;
			}
		} while (m_heartbeat_ack.load());
	}, interval);
}

void
cGateway::stop_heartbeating() {
	m_heartbeat_exit.store(true);
	if (m_heartbeat_thread.joinable())
		m_heartbeat_thread.join();
}