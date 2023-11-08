#include "GatewayImpl.h"
#include "Utils.h"
/* ========== Event type enum ======================================================================================= */
enum : uint32_t {
	RESUMED                       = 0x712ED6EE,
	READY                         = 0xDFC1471F,
	GUILD_CREATE                  = 0xDA68E698,
	GUILD_ROLE_CREATE             = 0xEFECD22C,
	GUILD_ROLE_UPDATE             = 0xF81F07AF,
	GUILD_ROLE_DELETE             = 0x5A284E50,
	GUILD_MEMBER_ADD              = 0x16401324,
	GUILD_MEMBER_UPDATE           = 0x73288C4D,
	GUILD_MEMBER_REMOVE           = 0x838DA405,
	INTERACTION_CREATE            = 0xB1E5D8EC,
	MESSAGE_CREATE                = 0x9C643E55,
	MESSAGE_UPDATE                = 0x8B97EBD6,
	MESSAGE_DELETE                = 0x29A0A229,
	MESSAGE_DELETE_BULK           = 0x2C44158B,
	MESSAGE_REACTION_ADD          = 0x5D4F2489,
	MESSAGE_REACTION_REMOVE       = 0x5295D7D8,
	MESSAGE_REACTION_REMOVE_ALL   = 0x02A1D6E9,
	MESSAGE_REACTION_REMOVE_EMOJI = 0xEB390E5D,
	GUILD_MEMBERS_CHUNK           = 0x343F4BC5,
	USER_UPDATE                   = 0xBFC98531
};
/* ========== Implement process_event() ============================================================================= */
void
cGateway::implementation::process_event(const json::value& v) try {
	/* Update last sequence received */
	m_last_sequence = v.at("s").as_int64();
	/* Determine the event type */
	auto name = json::value_to<std::string_view>(v.at("t"));
	auto type = cUtils::CRC32(0, name);
#ifdef GW_LOG_LVL_1
	cUtils::PrintLog("Event: 0x{:08X} {}", type, name);
#endif
	/* Trigger the appropriate event methods
	 * All JSON data is parsed BEFORE co_awaiting, as doing so will invalidate the argument reference! */
	switch (const json::value& d = v.at("d"); type) {
		case READY: {
			m_session_id = json::value_to<std::string>(d.at("session_id"));
			m_resume_gateway_url = json::value_to<std::string>(d.at("resume_gateway_url"));
			auto user = cHandle::MakeUnique<cUser>(d.at("user"));
			auto app = json::value_to<cApplication>(d.at("application"));
			co_await ResumeOnEventThread();
			m_application = std::move(app);
			co_await m_parent->OnReady(std::move(user));
			break;
		}
		case GUILD_CREATE: {
			auto pGuild = cHandle::MakeUnique<cGuild>(d);
			co_await ResumeOnEventThread();
			co_await m_parent->OnGuildCreate(std::move(pGuild));
			break;
		}
		case GUILD_ROLE_CREATE: {
			cSnowflake guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
			cRole role{ d.at("role") };
			co_await ResumeOnEventThread();
			co_await m_parent->OnGuildRoleCreate(guild_id, role);
			break;
		}
		case GUILD_ROLE_UPDATE: {
			cSnowflake guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
			cRole role{ d.at("role") };
			co_await ResumeOnEventThread();
			co_await m_parent->OnGuildRoleUpdate(guild_id, role);
			break;
		}
		case GUILD_ROLE_DELETE: {
			auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
			auto role_id = json::value_to<cSnowflake>(d.at("role_id"));
			co_await ResumeOnEventThread();
			co_await m_parent->OnGuildRoleDelete(guild_id, role_id);
			break;
		}
		case GUILD_MEMBER_ADD: {
			auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
			cMember member{ d };
			co_await ResumeOnEventThread();
			co_await m_parent->OnGuildMemberAdd(guild_id, member);
			break;
		}
		case GUILD_MEMBER_UPDATE: {
			auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
			cPartialMember member{ d };
			co_await ResumeOnEventThread();
			co_await m_parent->OnGuildMemberUpdate(guild_id, member);
			break;
		}
		case GUILD_MEMBER_REMOVE: {
			auto guild_id = json::value_to<cSnowflake>(d.at("guild_id"));
			cUser user{ d.at("user") };
			co_await ResumeOnEventThread();
			co_await m_parent->OnGuildMemberRemove(guild_id, user);
			break;
		}
		case INTERACTION_CREATE: {
			switch (d.at("type").to_number<int>()) {
				case INTERACTION_APPLICATION_COMMAND: {
					cApplicationCommandInteraction i(d);
					co_await ResumeOnEventThread();
					co_await m_parent->OnInteractionCreate(i);
					break;
				}
				case INTERACTION_MESSAGE_COMPONENT: {
					cMessageComponentInteraction i(d.as_object(), d.at("data").as_object());
					co_await ResumeOnEventThread();
					co_await m_parent->OnInteractionCreate(i);
					break;
				}
				case INTERACTION_MODAL_SUBMIT: {
					cModalSubmitInteraction i(d.as_object(), d.at("data").as_object());
					co_await ResumeOnEventThread();
					co_await m_parent->OnInteractionCreate(i);
					break;
				}
				default:
					cUtils::PrintLog("Unimplemented interaction type");
					break;
			}
			break;
		}
		case MESSAGE_CREATE: {
			auto& o = d.as_object();
			std::optional<cSnowflake> opt1;
			std::optional<cMember> opt2;
			cMessage m{ d };
			hSnowflake guild_id;
			hMember member;
			if (auto p = o.if_contains("guild_id"))
				guild_id = &opt1.emplace(json::value_to<std::string_view>(*p));
			if (auto p = o.if_contains("member"))
				member = &opt2.emplace(*p);
			co_await ResumeOnEventThread();
			co_await m_parent->OnMessageCreate(m, guild_id, member);
			break;
		}
		case MESSAGE_DELETE: {
			auto id = json::value_to<cSnowflake>(d.at("id"));
			auto channel_id = json::value_to<cSnowflake>(d.at("channel_id"));
			std::optional<cSnowflake> guild_id;
			if (auto p = d.as_object().if_contains("guild_id"))
				guild_id.emplace(json::value_to<std::string_view>(*p));
			co_await ResumeOnEventThread();
			co_await m_parent->OnMessageDelete(id, channel_id, guild_id.has_value() ? &*guild_id : nullptr);
			break;
		}
		case MESSAGE_DELETE_BULK: {
			auto ids = json::value_to<std::vector<cSnowflake>>(d.at("ids"));
			auto channel_id = json::value_to<cSnowflake>(d.at("channel_id"));
			std::optional<cSnowflake> guild_id;
			if (auto p = d.as_object().if_contains("guild_id"))
				guild_id.emplace(json::value_to<std::string_view>(*p));
			co_await ResumeOnEventThread();
			co_await m_parent->OnMessageDeleteBulk(ids, channel_id, guild_id.has_value() ? &*guild_id : nullptr);
			break;
		}
		case MESSAGE_REACTION_ADD: {
			auto& o = d.as_object();
			auto user_id = json::value_to<cSnowflake>(o.at("user_id"));
			auto channel_id = json::value_to<cSnowflake>(o.at("channel_id"));
			auto message_id = json::value_to<cSnowflake>(o.at("message_id"));
			cEmoji emoji(o.at("emoji"));
			hSnowflake guild_id, author_id;
			hMember member;
			std::optional<cSnowflake> opt1, opt2;
			std::optional<cMember> opt3;
			if (auto p = o.if_contains("guild_id"))
				guild_id = &opt1.emplace(json::value_to<std::string_view>(*p));
			if (auto p = o.if_contains("message_author_id"))
				author_id = &opt2.emplace(json::value_to<std::string_view>(*p));
			if (auto p = o.if_contains("member"))
				member = &opt3.emplace(*p);
			co_await ResumeOnEventThread();
			co_await m_parent->OnMessageReactionAdd(user_id, channel_id, message_id, guild_id, author_id, member, emoji);
			break;
		}
		case MESSAGE_REACTION_REMOVE: {
			auto& o = d.as_object();
			auto user_id = json::value_to<cSnowflake>(o.at("user_id"));
			auto channel_id = json::value_to<cSnowflake>(o.at("channel_id"));
			auto message_id = json::value_to<cSnowflake>(o.at("message_id"));
			cEmoji emoji(o.at("emoji"));
			hSnowflake guild_id;
			std::optional<cSnowflake> opt;
			if (auto p = o.if_contains("guild_id"))
				guild_id = &opt.emplace(json::value_to<std::string_view>(*p));
			co_await ResumeOnEventThread();
			co_await m_parent->OnMessageReactionRemove(user_id, channel_id, message_id, guild_id, emoji);
			break;
		}
		case MESSAGE_REACTION_REMOVE_ALL: {
			auto& o = d.as_object();
			auto channel_id = json::value_to<cSnowflake>(o.at("channel_id"));
			auto message_id = json::value_to<cSnowflake>(o.at("message_id"));
			hSnowflake guild_id;
			std::optional<cSnowflake> opt;
			if (auto p = o.if_contains("guild_id"))
				guild_id = &opt.emplace(json::value_to<std::string_view>(*p));
			co_await ResumeOnEventThread();
			co_await m_parent->OnMessageReactionRemoveAll(channel_id, message_id, guild_id);
			break;
		}
		case MESSAGE_REACTION_REMOVE_EMOJI: {
			auto& o = d.as_object();
			auto channel_id = json::value_to<cSnowflake>(o.at("channel_id"));
			auto message_id = json::value_to<cSnowflake>(o.at("message_id"));
			cEmoji emoji(o.at("emoji"));
			hSnowflake guild_id;
			std::optional<cSnowflake> opt;
			if (auto p = o.if_contains("guild_id"))
				guild_id = &opt.emplace(json::value_to<std::string_view>(*p));
			co_await ResumeOnEventThread();
			co_await m_parent->OnMessageReactionRemoveEmoji(channel_id, message_id, guild_id, emoji);
			break;
		}
		case GUILD_MEMBERS_CHUNK: {
			cGuildMembersChunk c{ d };
			co_await ResumeOnEventThread();
			auto& r = m_rgm_map[c.GetNonce()];
			r.Fill(std::move(c));
			break;
		}
		case USER_UPDATE: {
			auto user = cHandle::MakeUnique<cUser>(d);
			m_application = json::value_to<cApplication>(co_await DiscordGet("/oauth2/applications/@me"));
			co_await m_parent->OnUserUpdate(std::move(user));
			break;
		}
		default:
			break;
	}
} catch (...) {
	try {
		std::rethrow_exception(std::current_exception());
	} catch (const std::exception& e) {
		cUtils::PrintErr("BONK! {}", e.what());
	} catch (...) {
		cUtils::PrintErr("BONK!");
	}

	/* If an exception occurs, close the stream */
	asio::dispatch(m_ws_strand, [this] () { if (m_ws) close(); });
}