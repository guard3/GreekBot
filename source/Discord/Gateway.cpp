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

cGateway::cGateway(const char* token) {
	strncpy(m_token, token, 59);
	m_token[59] = '\0';
	m_ws.SetOnConnect([this]() {
		OnConnect();
	});
}

int cGateway::GetLastSequence() {
	m_last_sequence.mutex.lock();
	int r = m_last_sequence.value;
	m_last_sequence.mutex.unlock();
	return r;
}

void cGateway::SetLastSequence(int v) {
	if (v) {
		m_last_sequence.mutex.lock();
		m_last_sequence.value = v;
		m_last_sequence.mutex.unlock();
	}
}

hEvent cGateway::GetEvent() {
	try {
		beast::flat_buffer b;
		m_ws.Read(b);
		printf("%.*s\n", (int)b.size(), b.data().data());
		json::parser p;
		p.write(reinterpret_cast<const char*>(b.data().data()), b.size());
		return std::make_unique<cEvent>(p.release());
	}
	catch (const std::exception&) {
		return hEvent();
	}
}

bool cGateway::SendHeartbeat() {
	/* Variables */
	int               s; // The last event sequence
	std::string       p; // The payload string
	beast::error_code e; // The error code of websocket
	
	/* Prepare payload string */
	if ((s = GetLastSequence())) {
		p.resize(30);
		int len = sprintf(p.data(), "{\"op\":1,\"d\":%d}", s);
		p.resize(len);
	}
	else p = "{\"op\":1,\"d\":null}";
	
	/* Send payload */
	m_heartbeat.mutex.lock();
	m_heartbeat.acknowledged = false;
	m_ws.Write(net::buffer(p), e);
	m_heartbeat.mutex.unlock();
	
	/* Check for error */
	return !static_cast<bool>(e);
}

bool cGateway::HeartbeatAcknowledged() {
	m_heartbeat.mutex.lock();
	bool r = m_heartbeat.acknowledged;
	m_heartbeat.mutex.unlock();
	return r;
}

void cGateway::AcknowledgeHeartbeat() {
	m_heartbeat.mutex.lock();
	m_heartbeat.acknowledged = true;
	m_heartbeat.mutex.unlock();
}

void cGateway::StartHeartbeating(int interval) {
	m_heartbeat.thread = std::thread([this](int interval) {
		/* Wait for a random amount of time */
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(interval * cUtils::Random())));
		
		for (;;) {
			/* Send a heartbeat*/
			if (!SendHeartbeat()) {
				cUtils::PrintErr("Couldn't send heartbeat");
				return;
			}
			
			/* Wait for the specified interval in milliseconds */
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			
			/* Make sure that heartbeat has been acknowledged */
			if (!HeartbeatAcknowledged()) {
				cUtils::PrintLog("Heartbeat not acknowledged");
				return;
			}
		}
	}, interval);
}

bool cGateway::Identify() {
	/* Prepare payload string */
	std::string s(200, '\0');
	int len = sprintf(s.data(), "{\"op\":2,\"d\":{\"token\":\"%s\",\"intents\":%d,\"properties\":{\"$os\":\"%s\",\"$browser\":\"GreekBot\",\"$device\":\"GreekBot\"}}}", m_token, 513, cUtils::GetOS());
	s.resize(len);
	
	/* Send payload */
	beast::error_code e;
	m_ws.Write(net::buffer(s), e);
	return !static_cast<bool>(e);
}

cGateway::~cGateway() {
	if (m_heartbeat.thread.joinable())
		m_heartbeat.thread.join();
}

void cGateway::OnConnect() {
	for (;;) {
		/* Get the next event from the gateway */
		hEvent event = GetEvent();
		if (!event)
			return;
		/* Update last sequence */
		SetLastSequence(event->GetSequence());
		/* Handle event based on type */
		switch (event->GetType()) {
			case EVENT_HEARTBEAT:
				if (!SendHeartbeat()) {
					cUtils::PrintErr("Couldn't send heartbeat");
					return;
				}
				break;
				
			case EVENT_HELLO: {
				hHelloEvent e = event->GetHelloData();
				if (!e) {
					cUtils::PrintErr("Invalid opcode %d received", event->GetType());
					return;
				}
				StartHeartbeating(e->GetHeartbeatInterval());
				Identify();
				break;
			}
				
			case EVENT_HEARTBEAT_ACK:
				AcknowledgeHeartbeat();
				break;
			
			case EVENT_READY: {
				hReadyEvent e = event->GetReadyData();
				if (e) {
					cUtils::PrintLog("Gateway version %d", e->GetVersion());
					strcpy(m_sessionId, e->GetSessionId());
					cUtils::PrintLog("Session ID: %s", m_sessionId);
				}
				else
					cUtils::PrintErr("Invalid READY event");
				break;
			}
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
			m_ws.Run((std::string(g->GetUrl()) + "/?v=9").c_str());
		}
	}
}
