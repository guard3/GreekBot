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

void cGateway::OnHandshake() {
   beast::flat_buffer b;
   beast::error_code e;
   Read(b, e);
   printf("%.*s", (int)b.data().size(), b.data().data());
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
