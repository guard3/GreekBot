#include "GatewayImpl.h"

void
cGateway::implementation::resume() {
	send(cUtils::Format(R"({"op":6,"d":{"token":"%s","session_id":"%s","seq":%)" PRIi64 "}}", GetToken(), m_session_id, m_last_sequence));
}

void
cGateway::implementation::identify() {
	/* Reset everything related to RequestGuildMembers command in case smth got stuck */
	asio::post(m_http_ioc, [this](){ rgm_reset(); });
	/* Then send the payload */
	send(cUtils::Format(R"({"op":2,"d":{"token":"%s","intents":%d,"compress":true,"properties":{"os":"%s","browser":"GreekBot","device":"GreekBot"}}})", GetToken(), m_intents, cUtils::GetOS()));
}

void
cGateway::implementation::heartbeat() {
	m_heartbeat_ack = false;
	if (m_last_sequence)
		send(cUtils::Format(R"({"op":1,"d":%)" PRId64 "}", m_last_sequence));
	else
		send(R"({"op":1,"d":null})");
}