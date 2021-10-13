#include "Gateway.h"
#include "Discord.h"
#include "Utils.h"
#include "Websocket.h"

cGateway::cGateway(const char* token) {
	strncpy(m_token, token, 59);
	m_token[59] = '\0';
}

bool cGateway::SendHeartbeat() {
	bool result;

	m_heartbeat.mutex.lock();
	m_heartbeat.acknowledged = false;
	volatile int s = m_last_sequence;
	if (s) {
		char payload[40];
		size_t size = sprintf(payload, R"({"op":1,"d":%d})", s);
		result = size == m_pWebsocket->Write(payload, size);
	}
	else result = 17 == m_pWebsocket->Write(R"({"op":1,"d":null})", 17);
	m_heartbeat.mutex.unlock();

	return result;
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
	bool result = false;
	if (char* payload = reinterpret_cast<char*>(malloc(200))) {
		// TODO: Parametrize intents
		size_t len = sprintf(payload, R"({"op":2,"d":{"token":"%s","intents":%d,"properties":{"$os":"%s","$browser":"GreekBot","$device":"GreekBot"}}})", m_token, 513, cUtils::GetOS());
		result = len == m_pWebsocket->Write(payload, len);
		free(payload);
	}
	return result;
}

bool cGateway::Resume() {
	bool result = false;
	if (char* payload = reinterpret_cast<char*>(malloc(200))) {
		size_t len = sprintf(payload, R"({"op":6,"d":{"token":"%s","session_id":"%s","seq":%d}})", m_token, m_sessionId, m_last_sequence);
		result = len == m_pWebsocket->Write(payload, len);
		free(payload);
	}
	return result;
}

cGateway::~cGateway() {
	delete[] m_sessionId;
	StopHeartbeating();
}

bool cGateway::OnEvent(chEvent event) {
	/* First, update event sequence */
	m_last_sequence = event->GetSequence();

	/* Then handle the event */
	switch (event->GetType()) {
		case EVENT_HEARTBEAT:
			/* Send a heartbeat */
			return SendHeartbeat();
			
		case EVENT_RECONNECT:
			/* Forcefully exit */
			return false;
			
		case EVENT_INVALID_SESSION:
			/* Wait a random amount of time between 1 and 5 seconds */
			std::this_thread::sleep_for(std::chrono::milliseconds(cUtils::Random(1000, 5000)));
			/* If session is resumable, try to resume */
			if (event->GetData<EVENT_INVALID_SESSION>()) return Resume();
			/* If not, then reset session and identify */
			m_last_sequence = 0;
			delete[] m_sessionId;
			m_sessionId = nullptr;
			return Identify();
			
		case EVENT_HELLO:
			if (int interval = event->GetData<EVENT_HELLO>(); interval >= 0) {
				/* Start heartbeating */
				StartHeartbeating(interval);
				/* If there is an active session, try to resume */
				return m_sessionId ? Resume() : Identify();
			}
			cUtils::PrintErr("Invalid HELLO event");
			return false;
			
		case EVENT_HEARTBEAT_ACK:
			/* Heartbeat was acknowledged */
			AcknowledgeHeartbeat();
			break;
		
		case EVENT_READY:
			if (auto e = event->GetData<EVENT_READY>()) {
				if (auto user = e->GetUser()) {
					if (auto session = e->GetSessionId()) {
						m_sessionId = session.release();
						if (m_onReady) m_onReady(std::move(user));
						break;
					}
				}
			}
			cUtils::PrintErr("Invalid READY event");
			return false;
			
		case EVENT_INTERACTION_CREATE:
			if (auto e = event->GetData<EVENT_INTERACTION_CREATE>()) {
				if (m_onInteractionCreate) m_onInteractionCreate(e.get());
				break;
			}
			cUtils::PrintErr("Invalid INTERACTION_CREATE event");
			return false;

		default:
			break;
	}
	return true;
}

void cGateway::Run() {
	char url[100];

	for (uchGatewayInfo g;;) {
		/* Get gateway info */
		for (int timeout = 5;; timeout += 5) {
			uchError error;
			g = cDiscord::GetGatewayInfo(m_http_auth, error);
			if (error) {
				cUtils::PrintErr("Couldn't retrieve gateway info");
				cUtils::PrintErr("Code   : %d", error->GetCode());
				cUtils::PrintErr("Message: %s", error->GetMessage());
				return;
			}
			if (g) break;

			if (timeout > 15) {
				fputc('\n', stderr);
				return;
			}
			if (timeout < 10) cUtils::PrintErr("Couldn't retrieve gateway info");
			int tick = timeout;
			do {
				fprintf(stderr, "[ERR] Retrying in %ds \r", tick);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			} while (tick--);
		};

		/* Prepare url string */
		sprintf(url, "%s/?v=%d&encoding=json", g->GetUrl(), DISCORD_API_VERSION);

		/* Create a websocket */
		cWebsocket websocket(url);
		m_pWebsocket = &websocket;

		/* Allocate buffer for receiving websocket messages */
		size_t recv_size = 256;
		char* recv_buffer;
		if (!(recv_buffer = reinterpret_cast<char*>(malloc(recv_size)))) {
			cUtils::PrintErr("Out of memory.");
			return;
		}

		/* Start listening for messages */
		for (size_t total_size, msg_size; websocket.IsOpen();) {
			total_size = 0;
			for (;;) {
				/* Read available message bytes */
				msg_size = websocket.Read(recv_buffer + total_size, recv_size - total_size);
				/* If an error occurred, close websocket and reconnect */
				if (msg_size == 0) {
					websocket.Close();
					goto LABEL_EXIT_LOOP;
				}
				/* Update total message size */
				total_size += msg_size;
				/* If the entire message has been read, proceed */
				if (websocket.IsMessageDone())
					break;
				/* Otherwise, if recv_buffer is full, reallocate more memory */
				if (total_size == recv_size) {
					void* temp = realloc(recv_buffer, recv_size <<= 1);
					if (!temp) {
						cUtils::PrintErr("Out of memory.");
						free(recv_buffer);
						websocket.Close();
						StopHeartbeating();
						return;
					}
					recv_buffer = reinterpret_cast<char*>(temp);
				}
			}

			try {
				/* Parse message as a JSON */
				json::monotonic_resource mr;
				json::stream_parser p(&mr);
				p.write(recv_buffer, total_size);
				/* Create event object from JSON */
				cEvent event(p.release());
				/* Handle event */
				if (!OnEvent(&event))
					break;
			}
			catch (const std::exception& e) {
				cUtils::PrintErr("Couldn't read incoming event. %s", e.what());
				break;
			}
		}
	LABEL_EXIT_LOOP:
		free(recv_buffer);
		StopHeartbeating();
	}
}
