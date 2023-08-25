#include "GatewayImpl.h"
#include "Utils.h"
/* ========== Event type enum ======================================================================================= */
enum eEvent : uint32_t {
	EVENT_RESUMED                   = 0x712ED6EE,
	EVENT_READY                     = 0xDFC1471F,
	EVENT_GUILD_CREATE              = 0xDA68E698,
	EVENT_GUILD_ROLE_CREATE         = 0xEFECD22C,
	EVENT_GUILD_ROLE_UPDATE         = 0xF81F07AF,
	EVENT_GUILD_ROLE_DELETE         = 0x5A284E50,
	EVENT_GUILD_JOIN_REQUEST_DELETE = 0xC3A463EA,
	EVENT_GUILD_MEMBER_ADD          = 0x16401324,
	EVENT_GUILD_MEMBER_UPDATE       = 0x73288C4D,
	EVENT_GUILD_MEMBER_REMOVE       = 0x838DA405,
	EVENT_INTERACTION_CREATE        = 0xB1E5D8EC,
	EVENT_MESSAGE_CREATE            = 0x9C643E55,
	EVENT_MESSAGE_UPDATE            = 0x8B97EBD6,
	EVENT_MESSAGE_DELETE            = 0x29A0A229,
	EVENT_GUILD_MEMBERS_CHUNK       = 0x343F4BC5,
	EVENT_USER_UPDATE               = 0xBFC98531
};
/* ========== Make void a valid coroutine return type =============================================================== */
template<>
struct std::coroutine_traits<void, cGateway::implementation&, const json::value&> {
	struct promise_type {
		void   get_return_object() const noexcept {}
		void unhandled_exception() const noexcept {}
		void         return_void() const noexcept {}
		std::suspend_never initial_suspend() const noexcept { return {}; }
		std::suspend_never   final_suspend() const noexcept { return {}; }
	};
};
/* ========== Implement process_event() ============================================================================= */
void
cGateway::implementation::process_event(const json::value& v) {
	try {
		/* Update last sequence received */
		m_last_sequence = v.at("s").as_int64();
		/* Determine the event type */
		auto name = json::value_to<std::string_view>(v.at("t"));
		auto type = crc32(0, reinterpret_cast<const Byte*>(name.data()), name.size());
#ifdef GW_LOG_LVL_1
		cUtils::PrintLog("Event: 0x{:08X} {}", type, name);
#endif
		/* Trigger the appropriate event methods
		 * All JSON data is parsed BEFORE co_awaiting, as doing so will invalidate the argument reference! */
		switch (const json::value& d = v.at("d"); type) {
			case EVENT_READY: {
				m_session_id = json::value_to<std::string>(d.at("session_id"));
				m_resume_gateway_url = json::value_to<std::string>(d.at("resume_gateway_url"));
				auto user = cHandle::MakeUnique<cUser>(d.at("user"));
				auto app = json::value_to<cApplication>(d.at("application"));
				co_await ResumeOnEventThread();
				m_application = std::move(app);
				co_await m_parent->OnReady(std::move(user));
				break;
			}
			case EVENT_GUILD_CREATE: {
				auto pGuild = cHandle::MakeUnique<cGuild>(d);
				co_await ResumeOnEventThread();
				co_await m_parent->OnGuildCreate(std::move(pGuild));
				break;
			}
			case EVENT_GUILD_ROLE_CREATE: {
				cSnowflake guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
				cRole role{ d.at("role") };
				co_await ResumeOnEventThread();
				co_await m_parent->OnGuildRoleCreate(guild_id, role);
				break;
			}
			case EVENT_GUILD_ROLE_UPDATE: {
				cSnowflake guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
				cRole role{ d.at("role") };
				co_await ResumeOnEventThread();
				co_await m_parent->OnGuildRoleUpdate(guild_id, role);
				break;
			}
			case EVENT_GUILD_ROLE_DELETE: {
				auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
				auto role_id = json::value_to<cSnowflake>(d.at("role_id"));
				co_await ResumeOnEventThread();
				co_await m_parent->OnGuildRoleDelete(guild_id, role_id);
				break;
			}
			case EVENT_GUILD_MEMBER_ADD: {
				auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
				cMember member{ d };
				co_await ResumeOnEventThread();
				co_await m_parent->OnGuildMemberAdd(guild_id, member);
				break;
			}
			case EVENT_GUILD_MEMBER_UPDATE: {
				auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
				cPartialMember member{ d };
				co_await ResumeOnEventThread();
				co_await m_parent->OnGuildMemberUpdate(guild_id, member);
				break;
			}
			case EVENT_GUILD_MEMBER_REMOVE: {
				auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
				cUser user{ d.at("user") };
				co_await ResumeOnEventThread();
				co_await m_parent->OnGuildMemberRemove(guild_id, user);
				break;
			}
			case EVENT_INTERACTION_CREATE: {
				cInteraction i{ d };
				co_await ResumeOnEventThread();
				co_await m_parent->OnInteractionCreate(i);
				break;
			}
			case EVENT_MESSAGE_CREATE: {
				cMessage m{ d };
				co_await ResumeOnEventThread();
				co_await m_parent->OnMessageCreate(m);
				break;
			}
			case EVENT_GUILD_MEMBERS_CHUNK: {
				cGuildMembersChunk c{ d };
				co_await ResumeOnEventThread();
				auto& r = m_rgm_map[c.GetNonce()];
				r.Fill(std::move(c));
				break;
			}
			case EVENT_USER_UPDATE: {
				auto user = cHandle::MakeUnique<cUser>(d);
				m_application = json::value_to<cApplication>(co_await DiscordGet("/oauth2/applications/@me"));
				co_await m_parent->OnUserUpdate(std::move(user));
				break;
			}
			default:
				break;
		}
	}
	catch (...) {
		/* Make sure to schedule the caught exception to be rethrown at the websocket thread */
		asio::post(m_ws_ioc, [e = std::current_exception()]() { std::rethrow_exception(e); });
	}
}