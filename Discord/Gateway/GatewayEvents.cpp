#include "GatewayImpl.h"
#include "Event.h"

void
cGateway::implementation::on_event(const cEvent& event) {
	/* Update last sequence received */
	m_last_sequence.store(event.GetSequence());
#ifdef GW_LOG_LVL_1
	cUtils::PrintLog("Event: 0x%08X %s", event.GetType(), event.GetName());
#endif
	/* Trigger event functions */
	switch (event.GetType()) {
		case EVENT_READY:
			if (auto e = event.GetData<EVENT_READY>()) {
				m_session_id = std::move(e->session_id);
				m_parent->OnReady(std::move(e->user));
				return;
			}
			break;
		case EVENT_GUILD_CREATE:
			if (auto e = event.GetData<EVENT_GUILD_CREATE>()) {
				asio::post(m_http_ioc, [this, e = std::move(e)]() mutable -> cTask<> {
					co_await m_parent->OnGuildCreate(std::move(e));
				});
				return;
			}
			break;

		case EVENT_GUILD_ROLE_CREATE:
			if (auto e = event.GetData<EVENT_GUILD_ROLE_CREATE>()) {
				asio::post(m_http_ioc, [this, e = std::move(e)]() mutable -> cTask<> {
					auto p = std::move(e);
					co_await m_parent->OnGuildRoleCreate(p->guild_id, p->role);
				});
				return;
			}
			break;

		case EVENT_GUILD_ROLE_UPDATE:
			if (auto e = event.GetData<EVENT_GUILD_ROLE_UPDATE>()) {
				asio::post(m_http_ioc, [this, e = std::move(e)]() mutable -> cTask<> {
					auto p = std::move(e);
					co_await m_parent->OnGuildRoleUpdate(p->guild_id, p->role);
				});
				return;
			}
			break;

		case EVENT_GUILD_ROLE_DELETE:
			if (auto e = event.GetData<EVENT_GUILD_ROLE_DELETE>()) {
				asio::post(m_http_ioc, [this, e = std::move(e)]() mutable -> cTask<> {
					auto p = std::move(e);
					co_await m_parent->OnGuildRoleDelete(p->guild_id, p->role_id);
				});
				return;
			}
			break;

		case EVENT_INTERACTION_CREATE:
			if (auto e = event.GetData<EVENT_INTERACTION_CREATE>()) {
				asio::post(m_http_ioc, [this, e = std::move(e)]() mutable -> cTask<> {
					auto i = std::move(e);
					co_await m_parent->OnInteractionCreate(*i);
				});
				return;
			}
			break;

		case EVENT_MESSAGE_CREATE:
			if (auto e = event.GetData<EVENT_MESSAGE_CREATE>()) {
				m_task_manager.CreateTask([this, e = std::move(e)]() { m_parent->OnMessageCreate(e.get()); });
				return;
			}
			break;

		default:
			return;
	}
	throw std::runtime_error(cUtils::Format("Invalid %s event", event.GetName()));
}