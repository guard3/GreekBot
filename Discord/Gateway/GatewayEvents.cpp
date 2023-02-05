#include "GatewayImpl.h"
#include "Event.h"
/* ========== Make void a valid coroutine return type =============================================================== */
template<typename... Args>
struct std::coroutine_traits<void, Args...> {
	struct promise_type {
		void   get_return_object() const noexcept {}
		void unhandled_exception() const noexcept {}
		void         return_void() const noexcept {}
		std::suspend_never initial_suspend() const noexcept { return {}; }
		std::suspend_never   final_suspend() const noexcept { return {}; }
	};
};
/* ========== Implement on_event() ================================================================================== */
void
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
			m_application = e.application;
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
				r.Fill(std::move(e));
				break;
			}
			case EVENT_USER_UPDATE: {
				co_await m_parent->OnUserUpdate(event.GetDataPtr<EVENT_USER_UPDATE>());
				m_application = cApplication(co_await m_parent->DiscordGet("/oauth2/applications/@me"));
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