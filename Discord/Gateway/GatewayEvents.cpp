#include "GatewayImpl.h"
#include "Event.h"

cDetachedTask
cGateway::implementation::on_event(cEvent event) {
	try {
		/* Update last sequence received */
		m_last_sequence = event.GetSequence();
#ifdef GW_LOG_LVL_1
		cUtils::PrintLog("Event: 0x%08X %s", event.GetType(), event.GetName());
#endif
		/* On 'READY' save the session id before switching to the event thread */
		if (event.GetType() == EVENT_READY) {
			auto e = event.GetData<EVENT_READY>();
			m_session_id = std::move(e.session_id);
			co_await ResumeOnEventThread();
			co_return co_await m_parent->OnReady(std::move(e.user));
		}
		/* Switch to the event thread */
		co_await ResumeOnEventThread();
		/* Trigger the appropriate event methods */
		switch (event.GetType()) {
			case EVENT_GUILD_CREATE: {
				co_await m_parent->OnGuildCreate(event.GetDataPtr<EVENT_GUILD_CREATE>());
				break;
			}
			case EVENT_GUILD_ROLE_CREATE: {
				auto e = event.GetData<EVENT_GUILD_ROLE_CREATE>();
				co_await m_parent->OnGuildRoleCreate(e.guild_id, e.role);
				break;
			}
			case EVENT_GUILD_ROLE_UPDATE: {
				auto e = event.GetData<EVENT_GUILD_ROLE_UPDATE>();
				co_await m_parent->OnGuildRoleUpdate(e.guild_id, e.role);
				break;
			}
			case EVENT_GUILD_ROLE_DELETE: {
				auto e = event.GetData<EVENT_GUILD_ROLE_DELETE>();
				co_await m_parent->OnGuildRoleDelete(e.guild_id, e.role_id);
				break;
			}
			case EVENT_INTERACTION_CREATE: {
				co_await m_parent->OnInteractionCreate(event.GetData<EVENT_INTERACTION_CREATE>());
				break;
			}
			case EVENT_MESSAGE_CREATE: {
				co_await m_parent->OnMessageCreate(event.GetData<EVENT_MESSAGE_CREATE>());
				break;
			}
			case EVENT_GUILD_MEMBERS_CHUNK: {
				auto e = event.GetData<EVENT_GUILD_MEMBERS_CHUNK>();
				auto& r = m_rgm_map[e.GetNonce()];
				r.Insert(std::move(e));
				if (r.IsReady())
					r.Resume();
				break;
			}
			default:
				break;
		}
	}
	catch (...) {
		/* Make sure to schedule the caught exception to be rethrown at the websocket thread */
		asio::post(m_ws_ioc, [except = std::current_exception()]() { std::rethrow_exception(except); });
	}
}