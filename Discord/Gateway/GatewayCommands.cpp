#include "GatewayImpl.h"
#include "Utils.h"

void
cGateway::implementation::resume() {
	send(fmt::format(R"({{"op":6,"d":{{"token":{:?},"session_id":{:?},"seq":{}}}}})", GetToken(), m_session_id, m_last_sequence));
}

void
cGateway::implementation::identify() {
	/* Then send the payload */
	send(fmt::format(R"({{"op":2,"d":{{"token":{:?},"intents":{},"properties":{{"os":{:?},"browser":"GreekBot","device":"GreekBot"}}}}}})", GetToken(), (int)m_intents, cUtils::GetOS()));
	/* Cancel ongoing guild member requests */
	rgm_reset();
}

void
cGateway::implementation::heartbeat() {
	m_heartbeat_ack = false;
	if (m_last_sequence)
		send(fmt::format(R"({{"op":1,"d":{}}})", m_last_sequence));
	else
		send(R"({"op":1,"d":null})");
	/* Cancel any stuck guild member requests */
	rgm_timeout();
}