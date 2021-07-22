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

void cGateway::StartHeartbeating(int interval) {
	m_heartbeat.thread = std::thread([this](int interval) {
		std::string s;       // The payload string
		beast::error_code e; // The error code of websocket
		int sequence;        // The last event sequence
		for (;;) {
			/* Prepare payload string */
			if ((sequence = GetLastSequence())) {
				s.resize(50);
				int len = sprintf(s.data(), "{\"op\":1,\"d\":%d}", sequence);
				s.resize(len);
			}
			else
				s = "{\"op\":1,\"d\":null}";
			
			/* Set heartbeat as not acknowledged at first */
			m_heartbeat.mutex.lock();
			m_heartbeat.acknowledged = false;
			m_heartbeat.mutex.unlock();
			
			/* Send payload */
			m_ws.Write(net::buffer(s), e);
			if (e) {
				cUtils::PrintErr("Couldn't send heartbeat; %s", e.message().c_str());
				return;
			}
			cUtils::PrintLog("Heartbeat sent");
			
			/* Wait for the specified interval in milliseconds */
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			
			/* Check if heartbeat has been acknowledged */
			m_heartbeat.mutex.lock();
			bool acknowledged = m_heartbeat.acknowledged;
			m_heartbeat.mutex.unlock();
			if (!acknowledged) {
				cUtils::PrintLog("Heartbeat not acknowledged");
				return;
			}
		}
	}, interval);
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
	m_ws.Write(net::buffer(s), e);
	return !static_cast<bool>(e);
}

cGateway::~cGateway() {
	if (m_heartbeat.thread.joinable())
		m_heartbeat.thread.join();
}

void cGateway::OnConnect() {
	bool started = false;
	for (;;) {
		/* Get the next event from the gateway */
		hEvent event = GetEvent();
		if (!event)
			return;
		/* Update last sequence */
		SetLastSequence(event->GetSequence());
		/* Handle event based on type */
		switch (event->GetType()) {
			case EVENT_HELLO: {
				hHelloEvent e = event->GetHelloData();
				if (!e) {
					cUtils::PrintErr("Invalid opcode %d received", event->GetType());
					return;
				}
				StartHeartbeating(e->GetHeartbeatInterval());
				break;
			}
				
			case EVENT_HEARTBEAT_ACK:
				AcknowledgeHeartbeat();
				if (!started) {
					Identify();
					started = true;
					cUtils::PrintLog("Identified");
				}
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
