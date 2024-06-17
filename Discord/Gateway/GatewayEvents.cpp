#include "GatewayImpl.h"
#include "Interaction.h"
#include "Utils.h"
/* ========== Event type enum ======================================================================================= */
enum : std::uint32_t {
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
cGateway::implementation::process_event(const json::value& v) {
	bool bSuccess = false;  // Indicates whether the event payload was successfully parsed;
	std::string event_name; // The name of the event as a string
#define switch_strand() ResumeOnEventThread(); bSuccess = true
	try {
		/* Update last sequence received */
		m_last_sequence = v.at("s").as_int64();
		/* Determine the event type */
		event_name = json::value_to<std::string>(v.at("t"));
		auto  type = cUtils::CRC32(0, event_name);
#ifdef GW_LOG_LVL_1
		cUtils::PrintLog("Event: 0x{:08X} {}", type, event_name);
#endif
		/* Trigger the appropriate event methods
		 * All JSON data is parsed BEFORE co_awaiting, as doing so will invalidate the argument reference! */
		switch (const json::object& d = v.at("d").as_object(); type) {
			case RESUMED: {
				while (!m_pending.empty()) {
					m_pending.front().resume();
					m_pending.pop_front();
				}
			}	break;
			case READY: {
				m_session_id = json::value_to<std::string>(d.at("session_id"));
				m_resume_gateway_url = json::value_to<std::string>(d.at("resume_gateway_url"));
				cUser user{ d.at("user") };
				cApplication app{ d.at("application") };
				co_await switch_strand();
				m_application = std::move(app);
				co_await m_parent->OnReady(user);
			}	break;
			case GUILD_CREATE: {
				cGuild guild{ d };
				co_await switch_strand();
				co_await m_parent->OnGuildCreate(guild);
			}	break;
			case GUILD_ROLE_CREATE: {
				cSnowflake guild_id{ d.at("guild_id").as_string() };
				cRole role{ d.at("role") };
				co_await switch_strand();
				co_await m_parent->OnGuildRoleCreate(guild_id, role);
			}	break;
			case GUILD_ROLE_UPDATE: {
				cSnowflake guild_id{ d.at("guild_id").as_string() };
				cRole role{ d.at("role") };
				co_await switch_strand();
				co_await m_parent->OnGuildRoleUpdate(guild_id, role);
			}	break;
			case GUILD_ROLE_DELETE: {
				cSnowflake guild_id{ d.at("guild_id").as_string() };
				cSnowflake  role_id{ d.at( "role_id").as_string() };
				co_await switch_strand();
				co_await m_parent->OnGuildRoleDelete(guild_id, role_id);
			}	break;
			case GUILD_MEMBER_ADD: {
				cSnowflake guild_id{ d.at("guild_id").as_string() };
				cMember member{ d };
				co_await switch_strand();
				co_await m_parent->OnGuildMemberAdd(guild_id, member);
			}	break;
			case GUILD_MEMBER_UPDATE: {
				cSnowflake guild_id{ d.at("guild_id").as_string() };
				cMemberUpdate member{ d };
				co_await switch_strand();
				co_await m_parent->OnGuildMemberUpdate(guild_id, member);
			}	break;
			case GUILD_MEMBER_REMOVE: {
				cSnowflake guild_id{ d.at("guild_id").as_string() };
				cUser user{ d.at("user") };
				co_await switch_strand();
				co_await m_parent->OnGuildMemberRemove(guild_id, user);
			}	break;
			case INTERACTION_CREATE: {
				using namespace detail; // Expose the interaction type enum
				switch (d.at("type").to_number<int>()) {
					case INTERACTION_APPLICATION_COMMAND: {
						cAppCmdInteraction i{ d };
						co_await switch_strand();
						co_await m_parent->OnInteractionCreate(i);
					}	break;
					case INTERACTION_MESSAGE_COMPONENT: {
						cMsgCompInteraction i{ d };
						co_await switch_strand();
						co_await m_parent->OnInteractionCreate(i);
					}	break;
					case INTERACTION_MODAL_SUBMIT: {
						cModalSubmitInteraction i{ d };
						co_await switch_strand();
						co_await m_parent->OnInteractionCreate(i);
					}	break;
					default:
						cUtils::PrintLog("Unimplemented interaction type");
						break;
				}
			}	break;
			case MESSAGE_CREATE: {
				std::optional<cSnowflake> opt1;
				std::optional<cPartialMember> opt2;
				cMessage m{ d };
				hSnowflake guild_id;
				hPartialMember member;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt1.emplace(p->as_string());
				if (auto p = d.if_contains("member"))
					member = &opt2.emplace(*p);
				co_await switch_strand();
				co_await m_parent->OnMessageCreate(m, guild_id, member);
			}	break;
			case MESSAGE_UPDATE: {
				std::optional<cSnowflake> opt1;
				std::optional<cPartialMember> opt2;
				cMessageUpdate m{ d };
				hSnowflake guild_id;
				hPartialMember member;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt1.emplace(p->as_string());
				if (auto p = d.if_contains("member"))
					member = &opt2.emplace(*p);
				co_await switch_strand();
				co_await m_parent->OnMessageUpdate(m, guild_id, member);
			}	break;
			case MESSAGE_DELETE: {
				cSnowflake id{ d.at("id").as_string() };
				cSnowflake channel_id{ d.at("channel_id").as_string() };
				std::optional<cSnowflake> opt;
				hSnowflake guild_id;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt.emplace(p->as_string());
				co_await switch_strand();
				co_await m_parent->OnMessageDelete(id, channel_id, guild_id);
			}	break;
			case MESSAGE_DELETE_BULK: {
				auto ids = json::value_to<std::vector<cSnowflake>>(d.at("ids"));
				cSnowflake channel_id{ d.at("channel_id").as_string() };
				std::optional<cSnowflake> opt;
				hSnowflake guild_id;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt.emplace(p->as_string());
				co_await switch_strand();
				co_await m_parent->OnMessageDeleteBulk(ids, channel_id, guild_id);
			}	break;
			case MESSAGE_REACTION_ADD: {
				cSnowflake    user_id{ d.at(   "user_id").as_string() };
				cSnowflake channel_id{ d.at("channel_id").as_string() };
				cSnowflake message_id{ d.at("message_id").as_string() };
				cEmoji emoji{ d.at("emoji") };
				hSnowflake guild_id, author_id;
				hMember member;
				std::optional<cSnowflake> opt1, opt2;
				std::optional<cMember> opt3;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt1.emplace(p->as_string());
				if (auto p = d.if_contains("message_author_id"))
					author_id = &opt2.emplace(p->as_string());
				if (auto p = d.if_contains("member"))
					member = &opt3.emplace(*p);
				co_await switch_strand();
				co_await m_parent->OnMessageReactionAdd(user_id, channel_id, message_id, guild_id, author_id, member, emoji);
			}	break;
			case MESSAGE_REACTION_REMOVE: {
				cSnowflake    user_id{ d.at(   "user_id").as_string() };
				cSnowflake channel_id{ d.at("channel_id").as_string() };
				cSnowflake message_id{ d.at("message_id").as_string() };
				cEmoji emoji{ d.at("emoji") };
				hSnowflake guild_id;
				std::optional<cSnowflake> opt;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt.emplace(p->as_string());
				co_await switch_strand();
				co_await m_parent->OnMessageReactionRemove(user_id, channel_id, message_id, guild_id, emoji);
			}	break;
			case MESSAGE_REACTION_REMOVE_ALL: {
				cSnowflake channel_id{ d.at("channel_id").as_string() };
				cSnowflake message_id{ d.at("message_id").as_string() };
				hSnowflake guild_id;
				std::optional<cSnowflake> opt;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt.emplace(p->as_string());
				co_await switch_strand();
				co_await m_parent->OnMessageReactionRemoveAll(channel_id, message_id, guild_id);
			}	break;
			case MESSAGE_REACTION_REMOVE_EMOJI: {
				cSnowflake channel_id{ d.at("channel_id").as_string() };
				cSnowflake message_id{ d.at("message_id").as_string() };
				cEmoji emoji{ d.at("emoji") };
				hSnowflake guild_id;
				std::optional<cSnowflake> opt;
				if (auto p = d.if_contains("guild_id"))
					guild_id = &opt.emplace(p->as_string());
				co_await switch_strand();
				co_await m_parent->OnMessageReactionRemoveEmoji(channel_id, message_id, guild_id, emoji);
			}	break;
			case GUILD_MEMBERS_CHUNK: {
				auto it = m_rgm_entries.find(cUtils::ParseInt<std::uint64_t>(d.at("nonce").as_string()));
				/* If there's no entry with nonce, smth must have gone pretty wrong */
				if (it == m_rgm_entries.end()) {
					struct _ : std::exception {
						const char* what() const noexcept override { return "Unexpected chunk."; }
					}; throw _{};
				}
				/* Add the chunk to the end of the queue, saving any exception if it occurs */
				auto& entry = it->second;
				try {
					entry.chunks.emplace_back(d);
				} catch (...) {
					entry.except = std::current_exception();
				}
				/* Resume the coroutine to consume the chunks */
				if (entry.coro)
					entry.coro.resume();
			}	break;
			case USER_UPDATE: {
				cUser user{ d };
				m_application = json::value_to<cApplication>(co_await DiscordGet("/oauth2/applications/@me"));
				co_await switch_strand();
				co_await m_parent->OnUserUpdate(user);
			}	break;
			default:
				break;
		}
	} catch (...) {
		/* Prepare strings depending on whether the exception was thrown before or after an event handling */
		char s1[32];
		char s2[32] = " event";
		if (bSuccess) {
			std::strcpy(s1, "handling a");
		} else {
			std::strcpy(s1, "processing a");
			std::strcat(s2, " payload");
		}
		std::strcat(s1, [&event_name] {
			if (event_name.empty())
				return "n";
			if (char c = event_name.front(); c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U')
				return "n ";
			return " ";
		}());
		/* Log exception message */
		try {
			throw;
		} catch (const std::exception &e) {
			const char* what = e.what();
			cUtils::PrintErr("Unhandled exception while {}{}{}{}{}", s1, event_name, s2, *what ? ": " : "", what);
		} catch (...) {
			cUtils::PrintErr("Unhandled exception while {}{}{}{}{}", s1, event_name, s2, "", "");
		}
		/* Restart if the error occurred while parsing the event payload */
		if (!bSuccess)
			asio::defer(m_ws_strand, [this] { close(); });
	}
}