#include "Gateway.h"
#include "Event.h"

void
cGateway::on_event(const cEvent& event) {
	/* Update last sequence received */
	m_last_sequence.store(event.GetSequence());
#ifdef GW_LOG_LVL_1
	cUtils::PrintLog("Event: 0x%08X %s", event.GetType(), event.GetName());
#endif
	/* Trigger event functions */
	switch (event.GetType()) {
		case EVENT_READY:
			if (auto e = event.GetData<EVENT_READY>()) {
				m_session_id = e->GetSessionId();
				OnReady(e->GetUser());
				return;
			}
			break;
		case EVENT_GUILD_CREATE:
			if (auto e = event.GetData<EVENT_GUILD_CREATE>()) {
				m_task_manager.CreateTask([this](auto e) { OnGuildCreate(std::move(e)); }, std::move(e));
				//OnGuildCreate(std::move(e));
				return;
			}
			break;

		case EVENT_GUILD_ROLE_CREATE:
			if (auto e = event.GetData<EVENT_GUILD_ROLE_CREATE>()) {
				m_task_manager.CreateTask([this](auto e) { OnGuildRoleCreate(e->GetGuildId(), e->GetRole()); }, std::move(e));
				//OnGuildRoleCreate(e->GetGuildId(), e->GetRole());
				return;
			}
			break;

		case EVENT_GUILD_ROLE_UPDATE:
			if (auto e = event.GetData<EVENT_GUILD_ROLE_UPDATE>()) {
				m_task_manager.CreateTask([this](auto e) { OnGuildRoleUpdate(e->GetGuildId(), e->GetRole()); }, std::move(e));
				//OnGuildRoleUpdate(e->GetGuildId(), e->GetRole());
				return;
			}
			break;

		case EVENT_GUILD_ROLE_DELETE:
			if (auto e = event.GetData<EVENT_GUILD_ROLE_DELETE>()) {
				//OnGuildRoleDelete(e->GetGuildId(), e->GetRoleId());
				m_task_manager.CreateTask([this](auto e) { OnGuildRoleDelete(e->GetGuildId(), e->GetRoleId()); }, std::move(e));
				return;
			}
			break;

		case EVENT_INTERACTION_CREATE:
			if (auto e = event.GetData<EVENT_INTERACTION_CREATE>()) {
				m_task_manager.CreateTask([this, e = std::move(e)]() { OnInteractionCreate(e.get()); });
				//OnInteractionCreate(e.get());
				return;
			}
			break;

		case EVENT_MESSAGE_CREATE:
			if (auto e = event.GetData<EVENT_MESSAGE_CREATE>()) {
				//OnMessageCreate(e.get());
				m_task_manager.CreateTask([this, e = std::move(e)]() { OnMessageCreate(e.get()); });
				return;
			}
			break;

		default:
			return;
	}
	throw std::runtime_error(cUtils::Format("Invalid %s event", event.GetName()));
}