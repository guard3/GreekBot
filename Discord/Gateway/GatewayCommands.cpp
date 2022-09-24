#include "GatewayImpl.h"

void
cGateway::implementation::resume() {
	send(cUtils::Format(R"({"op":6,"d":{"token":"%s","session_id":"%s","seq":%)" PRIi64 "}}", GetToken(), m_session_id.c_str(), m_last_sequence.load()));
}

void
cGateway::implementation::identify() {
	send(cUtils::Format(R"({"op":2,"d":{"token":"%s","intents":%d,"compress":true,"properties":{"$os":"%s","$browser":"GreekBot","$device":"GreekBot"}}})", GetToken(), m_intents, cUtils::GetOS()));
}

void
cGateway::implementation::heartbeat() {
	m_heartbeat_ack.store(false);
	if (int64_t s = m_last_sequence)
		send(cUtils::Format(R"({"op":1,"d":%)" PRId64 "}", s));
	else
		send(R"({"op":1,"d":null})");
}

void
cGateway::implementation::start_heartbeating(chrono::milliseconds interval) {
	m_heartbeat_exit.store(false);
	m_heartbeat_thread = std::thread([this, interval]() {
		//auto interval = std::chrono::milliseconds(interval_int);
		/* Wait for a random amount of time */
		auto later = chrono::steady_clock::now() + chrono::milliseconds(cUtils::Random(0, interval.count()));
		do {
			if (m_heartbeat_exit.load())
				return;
		}
		while (chrono::steady_clock::now() < later);
		/* Begin the heartbeating loop */
		do {
			heartbeat();
			later = chrono::steady_clock::now() + interval;
			do {
				if (m_heartbeat_exit.load())
					return;
			}
			while (chrono::steady_clock::now() < later);
		} while (m_heartbeat_ack.load());
	});
}

void
cGateway::implementation::stop_heartbeating() {
	m_heartbeat_exit.store(true);
	if (m_heartbeat_thread.joinable())
		m_heartbeat_thread.join();
}