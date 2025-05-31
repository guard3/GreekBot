#include "GatewayImpl.h"
#include "Utils.h"

void
cGateway::implementation::resume(std::shared_ptr<websocket_session> sess) {
	send(std::move(sess), std::format(R"({{"op":6,"d":{{"token":{:?},"session_id":{:?},"seq":{}}}}})", GetToken(), m_session_id, m_last_sequence));
}

void
cGateway::implementation::identify(std::shared_ptr<websocket_session> sess) {
	/* Then send the payload */
	send(std::move(sess), std::format(R"({{"op":2,"d":{{"token":{:?},"intents":{},"properties":{{"os":{:?},"browser":"GreekBot","device":"GreekBot"}}}}}})", GetToken(), std::to_underlying(m_intents), cUtils::GetOS()));
	/* Cancel ongoing guild member requests */
	rgm_reset();
}

void
cGateway::implementation::heartbeat(std::shared_ptr<websocket_session> sess) {
	sess->hb_ack = false;
	send(std::move(sess), m_last_sequence ? std::format(R"({{"op":1,"d":{}}})", m_last_sequence) : R"({"op":1,"d":null})");
	/* Cancel any stuck guild member requests */
	rgm_timeout();
}

void
cGateway::implementation::on_heartbeat() {
	co_await ResumeOnEventStrand();
	co_await m_parent->OnHeartbeat();
}