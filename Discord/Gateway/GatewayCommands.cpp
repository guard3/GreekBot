#include "GatewayImpl.h"

void
cGateway::implementation::resume() {
	send(fmt::format(R"({{"op":6,"d":{{"token":{:?},"session_id":{:?},"seq":{}}}}})", GetToken(), m_session_id, m_last_sequence));
}

void
cGateway::implementation::identify() {
	/* Reset everything related to RequestGuildMembers command in case smth got stuck */
	asio::post(m_http_ioc, [this](){ rgm_reset(); });
	/* Then send the payload */
	send(fmt::format(R"({{"op":2,"d":{{"token":{:?},"intents":{},"compress":true,"properties":{{"os":{:?},"browser":"GreekBot","device":"GreekBot"}}}}}})", GetToken(), (int)m_intents, cUtils::GetOS()));
}

void
cGateway::implementation::heartbeat() {
	m_heartbeat_ack = false;
	if (m_last_sequence)
		send(fmt::format(R"({{"op":1,"d":{}}})", m_last_sequence));
	else
		send(R"({"op":1,"d":null})");
}