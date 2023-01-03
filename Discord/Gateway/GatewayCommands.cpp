#include "GatewayImpl.h"

void
cGateway::implementation::resume() {
	send(cUtils::Format(R"({"op":6,"d":{"token":"%s","session_id":"%s","seq":%)" PRIi64 "}}", GetToken(), m_session_id, m_last_sequence.load()));
}

void
cGateway::implementation::identify() {
	send(cUtils::Format(R"({"op":2,"d":{"token":"%s","intents":%d,"compress":true,"properties":{"os":"%s","browser":"GreekBot","device":"GreekBot"}}})", GetToken(), m_intents, cUtils::GetOS()));
}

void
cGateway::implementation::heartbeat() {
	m_heartbeat_ack = false;
	if (int64_t s = m_last_sequence)
		send(cUtils::Format(R"({"op":1,"d":%)" PRId64 "}", s));
	else
		send(R"({"op":1,"d":null})");
}

void
cGateway::implementation::start_heartbeating(chrono::milliseconds interval) {
	m_heartbeat_exit = false;
	m_heartbeat_thread = std::thread([this, interval]() {
		/* Wait for a random amount of time */
		auto later = chrono::steady_clock::now() + chrono::milliseconds(cUtils::Random(0, interval.count()));
		do {
			if (m_heartbeat_exit)
				return;
		}
		while (chrono::steady_clock::now() < later);
		/* Begin the heartbeating loop */
		do {
			heartbeat();
			later = chrono::steady_clock::now() + interval;
			do {
				if (m_heartbeat_exit)
					return;
			}
			while (chrono::steady_clock::now() < later);
		} while (m_heartbeat_ack);
	});
}

void
cGateway::implementation::stop_heartbeating() {
	m_heartbeat_exit = true;
	if (m_heartbeat_thread.joinable())
		m_heartbeat_thread.join();
}