#include "Gateway.h"
#include "Discord.h"
#include "Utils.h"

cGatewayInfo::cGatewayInfo(const char* auth) {
	json::value v;
	cDiscord::GetGateway(auth, v);
	const json::object& obj = v.as_object();
	try {
		const json::string& url = obj.at("url").as_string();
		if ((m_url = reinterpret_cast<char*>(malloc(url.size() + 1)))) {
			strcpy(m_url, url.c_str());
			m_shards = static_cast<int>(obj.at("shards").as_int64());
			m_ssl = new cSessionStartLimit(obj.at("session_start_limit").as_object());
		}
	}
	catch (const std::exception&) {
		m_err = new cJsonError(obj);
	}
}

hPayload cGateway::ReceivePayload() {
	try {
		beast::flat_buffer b;
		Read(b);
		json::parser p;
		p.write(reinterpret_cast<const char*>(b.data().data()), b.size());
		return std::make_unique<cPayload>(p.release());
	}
	catch (const std::exception&) {
		return hPayload();
	}
}

void test(cGateway* t, int in) {
	for (;;) {
		printf("BOO\n");
		t->Write(net::buffer(std::string("{\"op\":1,\"d\":null}")));
		std::this_thread::sleep_for(std::chrono::milliseconds(in));
	}
}

void cGateway::StartHeartbeating(int interval) {
	m_heartbeatThread = std::thread(test, this, interval);
}

void cGateway::OnHandshake() {
	for (;;) {
		hPayload payload = ReceivePayload();
		if (!payload)
			return;
		
		switch (payload->GetOpcode()) {
			case PAYLOAD_HELLO:
				printf("OPCODE HELLO: %d\n", payload->ToHelloPayload()->GetHeartbeatInterval());
				StartHeartbeating(payload->ToHelloPayload()->GetHeartbeatInterval());
				break;
			default:
				printf("OPCODE: %d\n", payload->GetOpcode());
				
		}
	}
}

void cGateway::Run(const char* auth) {
	hGatewayInfo g = GetGatewayInfo(auth);
	if (g) {
		if (g->GetError()) {
			cUtils::PrintErr("Retrieving gateway info. %s", g->GetError()->GetMessage());
		}
		else {
			cWebsocket::Run(g->GetUrl(), auth, 64);
		}
	}
}
