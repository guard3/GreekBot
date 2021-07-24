#include "Gateway.h"
#include "Discord.h"
#include "Utils.h"
#include "Websocket.h"

cGateway::cGateway(const char* token) {
	strncpy(m_token, token, 59);
	m_token[59] = '\0';
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

void cGateway::ResetSession() {
	m_last_sequence.mutex.lock();
	m_last_sequence.value = 0;
	m_last_sequence.mutex.unlock();
	m_sessionId[0] = '\0';
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
	m_pWebsocket->Write(net::buffer(p), e);
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
		/* Wait a random amount of time */
		m_heartbeat.should_exit = false;
		auto later = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<int>(interval * cUtils::Random()));
		while (std::chrono::system_clock::now() < later)
			if (m_heartbeat.should_exit)
				return;
		
		for (;;) {
			/* Send a heartbeat*/
			if (!SendHeartbeat()) {
				cUtils::PrintErr("Couldn't send heartbeat");
				return;
			}
			
			/* Wait for the specified interval in milliseconds */
			later = std::chrono::system_clock::now() + std::chrono::milliseconds(interval);
			while (std::chrono::system_clock::now() < later)
				if (m_heartbeat.should_exit)
					return;
			
			/* Make sure that heartbeat has been acknowledged */
			if (!HeartbeatAcknowledged()) {
				cUtils::PrintLog("Heartbeat not acknowledged");
				return;
			}
		}
	}, interval);
}

void cGateway::StopHeartbeating() {
	m_heartbeat.should_exit = true;
	if (m_heartbeat.thread.joinable())
		m_heartbeat.thread.join();
}

bool cGateway::Identify() {
	/* Prepare payload string */
	std::string s;
	s.resize(200);
	int len = sprintf(s.data(), "{\"op\":2,\"d\":{\"token\":\"%s\",\"intents\":%d,\"properties\":{\"$os\":\"%s\",\"$browser\":\"GreekBot\",\"$device\":\"GreekBot\"}}}", m_token, 513, cUtils::GetOS());
	s.resize(len);
	
	/* Send payload */
	beast::error_code e;
	m_pWebsocket->Write(net::buffer(s), e);
	return !static_cast<bool>(e);
}

bool cGateway::Resume() {
	/* Prepare payload string */
	std::string s;
	s.resize(200);
	int len = sprintf(s.data(), "{\"op\":6,\"d\":{\"token\":\"%s\",\"session_id\":\"%s\",\"seq\":%d}}", m_token, m_sessionId, m_last_sequence.value);
	s.resize(len);
	
	/* Send payload */
	beast::error_code e;
	m_pWebsocket->Write(net::buffer(s), e);
	return !static_cast<bool>(e);
}

cGateway::~cGateway() {
	StopHeartbeating();
}

bool cGateway::OnEvent(cEvent *event) {
	switch (event->GetType()) {
		case EVENT_HEARTBEAT:
			return SendHeartbeat();
			
		case EVENT_INVALID_SESSION: {
			auto e = event->GetData<EVENT_INVALID_SESSION>();
			if (!e) {
				cUtils::PrintErr("Invalid INVALID SESSION event");
				return false;
			}
			/* Wait a random amount of time between 1 and 5 seconds */
			std::this_thread::sleep_for(std::chrono::milliseconds(cUtils::Random(1000, 5000)));
			/* If session is resumable, try to Resume */
			if (e->IsSessionResumable()) return Resume();
			/* If not, then reset session and identify */
			ResetSession();
			return Identify();
		}
			
		case EVENT_HELLO: {
			auto e = event->GetData<EVENT_HELLO>();
			if (!e) {
				cUtils::PrintErr("Invalid HELLO event");
				return false;
			}
			/* Start heartbeating */
			StartHeartbeating(e->GetHeartbeatInterval());
			/* If there is an active session, try to resume */
			return m_sessionId[0] ? Resume() : Identify();
		}
			
		case EVENT_HEARTBEAT_ACK:
			AcknowledgeHeartbeat();
			break;
		
		case EVENT_READY: {
			auto e = event->GetData<EVENT_READY>();
			if (e) {
				cUtils::PrintLog("Gateway version %d", e->GetVersion());
				strcpy(m_sessionId, e->GetSessionId());
				cUtils::PrintLog("Session ID: %s", m_sessionId);
			}
			else {
				cUtils::PrintErr("Invalid READY event");
				return false;
			}
			break;
		}
	}
	return true;
}

void cGateway::Run() {
	char url[100];
	
	for (;;) {
		/* Get gateway info */
		hGatewayInfo g = cDiscord::GetGatewayInfo(m_http_auth);
		if (!g) {
			cUtils::PrintErr("Couldn't retrieve gateway info");
			for (int timeout = 5;; timeout += 5) {
				for (int tick = timeout;; --tick) {
					fprintf(stderr, "[ERR] Retrying in %ds \r", tick);
					std::this_thread::sleep_for(std::chrono::seconds(1));
					if (tick == 0)
						break;
				}
				if ((g = cDiscord::GetGatewayInfo(m_http_auth))) {
					fputc('\n', stderr);
					break;
				}
				if (timeout > 10) {
					fputc('\n', stderr);
					cUtils::PrintErr("Exiting...");
					return;
				}
			}
		}
		
		if (g->GetError())
			cUtils::PrintErr("Couldn't retrieve gateway info. %s", g->GetError()->GetMessage());
		else {
			/* Prepare url string */
			sprintf(url, "%s/?v=%d&encoding=json", g->GetUrl(), DISCORD_API_VERSION);
			
			/* Create a websocket */
			cWebsocket websocket;
			m_pWebsocket = &websocket;
			
			websocket.SetOnMessage([this](cWebsocket* ws, void* data, size_t size) {
				try {
					/* Parse message as a JSON */
					json::monotonic_resource mr;
					json::stream_parser p(&mr);
					p.write(reinterpret_cast<char*>(data), size);
					
					/* Create event object from JSON */
					cEvent event(p.release());
					
					/* Update event sequence */
					SetLastSequence(event.GetSequence());
					
					/* Handle event */
					if (OnEvent(&event))
						return;
				}
				catch (const std::exception& e) {
					cUtils::PrintErr("Couldn't read incoming event. %s", e.what());
				}
				ws->Close();
			}).Run(url);
			StopHeartbeating();
		}
		
		cDiscord::CloseHandle(g);
	}
}
