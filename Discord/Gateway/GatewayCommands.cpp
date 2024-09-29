#include "GatewayImpl.h"
#include "Utils.h"

void
cGateway::implementation::resume() {
	send(std::format(R"({{"op":6,"d":{{"token":"{}","session_id":"{}","seq":{}}}}})", GetToken(), m_session_id, m_last_sequence));
}

void
cGateway::implementation::identify() {
	/* Then send the payload */
	send(std::format(R"({{"op":2,"d":{{"token":"{}","intents":{},"properties":{{"os":"{}","browser":"GreekBot","device":"GreekBot"}}}}}})", GetToken(), (int)m_intents, cUtils::GetOS()));
	/* Cancel ongoing guild member requests */
	rgm_reset();
}

void
cGateway::implementation::heartbeat() {
	m_heartbeat_ack = false;
	if (m_last_sequence)
		send(std::format(R"({{"op":1,"d":{}}})", m_last_sequence));
	else
		send(R"({"op":1,"d":null})");
	/* Cancel any stuck guild member requests */
	rgm_timeout();
}

void
cGateway::implementation::on_heartbeat() try {
	co_await ResumeOnEventStrand();
	co_await m_parent->OnHeartbeat();
} catch (...) {
	try {
		throw;
	} catch (const std::exception& e) {
		cUtils::PrintErr("An unhandled exception escaped the heartbeat handler: {}", e.what());
	} catch (...) {
		cUtils::PrintErr("An unhandled exception escaped the heartbeat handler.");
	}
	net::post(m_http_strand, [ex = std::current_exception()] { std::rethrow_exception(ex); });
}