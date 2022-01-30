#include "Gateway.h"
#include "Utils.h"
#include "Websocket.h"
#include <iostream>
// Workaround to make this compile on Windows, YIKES!
// TODO: Rename I guess...
#undef GetMessage
uchGatewayInfo cGateway::get_gateway_info(cDiscordError& error) {
	try {
		/* The JSON parser */
		json::monotonic_resource mr;
		json::stream_parser p(&mr);
		/* The http request response */
		std::string http_response;
		/* Return gateway info if http request is successful */
		if (200 == cDiscord::GetHttpsRequest(DISCORD_API_GATEWAY_BOT, m_http_auth, http_response, error)) {
			p.write(http_response);
			return cHandle::MakeUnique<cGatewayInfo>(p.release());
		}
	}
	catch (...) {
		error = cDiscordError::MakeError<DISCORD_ERROR_GENERIC>();
	}
	return {};
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
	m_heartbeat.should_exit = false;
	m_heartbeat.thread = std::thread([this](int interval) {
		/* Wait a random amount of time */
		auto later = std::chrono::system_clock::now() + std::chrono::milliseconds((int)((float)interval * cUtils::Random()));
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
		size_t len = sprintf(payload, R"({"op":2,"d":{"token":"%s","intents":%d,"properties":{"$os":"%s","$browser":"GreekBot","$device":"GreekBot"}}})", m_http_auth + 4, m_intents, cUtils::GetOS());
		result = len == m_pWebsocket->Write(payload, len);
		free(payload);
	}
	return result;
}

bool cGateway::Resume() {
	bool result = false;
	if (char* payload = reinterpret_cast<char*>(malloc(200))) {
		size_t len = sprintf(payload, R"({"op":6,"d":{"token":"%s","session_id":"%s","seq":%d}})", m_http_auth + 4, m_sessionId, m_last_sequence);
		result = len == m_pWebsocket->Write(payload, len);
		free(payload);
	}
	return result;
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
						OnReady(std::move(user));
						break;
					}
				}
			}
			cUtils::PrintErr("Invalid READY event");
			return false;

		case EVENT_GUILD_CREATE:
			if (auto e = event->GetData<EVENT_GUILD_CREATE>()) {
				OnGuildCreate(e.get());
				break;
			}
			cUtils::PrintErr("Invalid GUILD_CREATE event");
			return false;
			
		case EVENT_INTERACTION_CREATE:
			if (auto e = event->GetData<EVENT_INTERACTION_CREATE>()) {
				OnInteractionCreate(e.get());
				break;
			}
			cUtils::PrintErr("Invalid INTERACTION_CREATE event");
			return false;

		case EVENT_MESSAGE_CREATE:
			if (auto e = event->GetData<EVENT_MESSAGE_CREATE>()) {
				OnMessageCreate(e.get());
				break;
			}
			cUtils::PrintErr("Invalid MESSAGE_CREATE event");
			return false;

		default:
			break;
	}
	return true;
}

void cGateway::Run() {
	/* The string that holds the gateway websocket url */
	char url[100];
	/* The gateway info object */
	uchGatewayInfo g;
	/* The buffer that holds the received messages from the gateway */
	size_t recv_size = 256;
	char  *recv_buff = reinterpret_cast<char*>(malloc(recv_size));
	if (!recv_buff) {
		cUtils::PrintErr("Out of memory");
		goto LABEL_RETURN_NO_FREE_BUFF;
	}
	/* The main loop */
	for (cDiscordError e;;) {
		/* Get gateway info - retry with a timeout if any error occurs */
		for (int timeout = 5, tick; ; timeout += 5) {
			g = get_gateway_info(e);
			if (!e) break;
			switch (timeout) {
				case 20:
					fputc('\n', stderr);
					goto LABEL_RETURN;
				case 5:
					cUtils::PrintErr("Couldn't retrieve gateway info");
				default:
					if (e.GetType() == DISCORD_ERROR_HTTP) {
						cUtils::PrintErr(e.Get()->GetMessage());
						cUtils::PrintErr("Error code: %d", e.Get()->GetCode());
						goto LABEL_RETURN;
					}
					tick = timeout;
					do {
						fprintf(stderr, "[ERR] Retrying in %ds \r", tick);
						std::this_thread::sleep_for(std::chrono::seconds(1));
					} while (tick--);
			}
		}
		/* Create a websocket */
		sprintf(url, "%s/?v=%d&encoding=json", g->GetUrl(), DISCORD_API_VERSION);
		cWebsocket websocket(url);
		m_pWebsocket = &websocket;
		/* Start listening for messages */
		while (websocket.IsOpen()) {
			size_t msg_size = 0;
			for (;;) {
				/* Read available message bytes */
				size_t recv_len = websocket.Read(recv_buff + msg_size, recv_size - msg_size);
				/* If an error occurred, reconnect */
				if (recv_len == 0) goto LABEL_EXIT_MESSAGE_LOOP;
				/* Update total message size */
				msg_size += recv_len;
				/* If the entire message has been read, proceed */
				if (websocket.IsMessageDone()) break;
				/* Otherwise, if the buffer is full, allocate more memory */
				if (msg_size == recv_size) {
					void* temp = realloc(recv_buff, recv_size <<= 1);
					if (!temp) {
						cUtils::PrintErr("Out of memory.");
						StopHeartbeating();
						goto LABEL_RETURN;
					}
					recv_buff = reinterpret_cast<char*>(temp);
				}
			}

			try {
				/* Parse message as a JSON */
				json::monotonic_resource mr;
				json::stream_parser p(&mr);
				p.write(recv_buff, msg_size);
				/* Create event object from JSON */
#if 0
				auto e = p.release();
				std::cout << e << std::endl;
				cEvent event(e);
#else
				cEvent event(p.release());
#endif
				/* Handle event */
				if (!OnEvent(&event)) break;
			}
			catch (const std::exception& e) {
				cUtils::PrintErr("Couldn't read incoming event. %s", e.what());
				break;
			}
		}
	LABEL_EXIT_MESSAGE_LOOP:
		StopHeartbeating();
	}
LABEL_RETURN:
	free(recv_buff);
LABEL_RETURN_NO_FREE_BUFF:
	cUtils::PrintLog("Exiting...");
}
