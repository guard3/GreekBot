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

cGateway::cGateway(const char* token) : cWebsocket() {
	strncpy(m_token, token, 59);
	m_token[59] = '\0';
}

hPayload cGateway::ReceivePayload() {
	try {
		beast::flat_buffer b;
		Read(b);
		printf("%.*s\n", b.size(), b.data().data());
		json::parser p;
		p.write(reinterpret_cast<const char*>(b.data().data()), b.size());
		return std::make_unique<cPayload>(p.release());
	}
	catch (const std::exception&) {
		return hPayload();
	}
}

void cGateway::StartHeartbeating(int interval) {
	m_heartbeat.thread = std::thread([](cGateway* gateway, int interval) {
		auto& heartbeat = gateway->m_heartbeat;
		beast::error_code e;
		for (;;) {
			/* Set heartbeat as not acknowledged at first */
			heartbeat.mutex.lock();
			heartbeat.acknowledged = false;
			heartbeat.mutex.unlock();
			
			/* Set heartbeat payload */
			gateway->Write(net::buffer(std::string("{\"op\":1,\"d\":null}")), e);
			if (e) {
				cUtils::PrintErr("Couldn't send heartbeat; %s", e.message().c_str());
				return;
			}
			cUtils::PrintLog("Heartbeat sent");
			
			/* Wait for the specified interval in milliseconds */
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			
			/* Check if heartbeat has been acknowledged */
			heartbeat.mutex.lock();
			bool acknowledged = heartbeat.acknowledged;
			heartbeat.mutex.unlock();
			if (!acknowledged) {
				cUtils::PrintLog("Heartbeat not acknowledged");
				return;
			}
		}
	}, this, interval);
}

void cGateway::AcknowledgeHeartbeat() {
	m_heartbeat.mutex.lock();
	m_heartbeat.acknowledged = true;
	m_heartbeat.mutex.unlock();
}

bool cGateway::Identify() {
	/* Prepare payload string */
	std::string s(200, '\0');
	int len = sprintf(s.data(), "{\"op\":2,\"d\":{\"token\":\"%s\",\"intents\":%d,\"properties\":{\"$os\":\"%s\",\"$browser\":\"GreekBot\",\"$device\":\"GreekBot\"}}}", m_token, 513, cUtils::GetOS());
	s.resize(len);
	
	/* Send payload */
	beast::error_code e;
	Write(net::buffer(s), e);
	return !static_cast<bool>(e);
}

cGateway::~cGateway() {
	if (m_heartbeat.thread.joinable())
		m_heartbeat.thread.join();
}

void cGateway::OnHandshake() {
	bool started = false;
	for (;;) {
		hPayload payload = ReceivePayload();
		if (!payload)
			return;
		
		switch (payload->GetOpcode()) {
			case PAYLOAD_HELLO: {
				hHelloPayload p = payload->ToHelloPayload();
				if (p) {
					cUtils::PrintLog("Received HELLO opcode");
					StartHeartbeating(p->GetHeartbeatInterval());
				}
				else
					cUtils::PrintErr("Invalid opcode received");
				break;
			}
				
			case PAYLOAD_HEARTBEAT_ACK:
				AcknowledgeHeartbeat();
				if (!started) {
					if (!Identify())
						cUtils::PrintErr("BOOOOO");
					started = true;
					cUtils::PrintLog("Identified");
				}
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
			cWebsocket::Run(g->GetUrl());
		}
	}
}
