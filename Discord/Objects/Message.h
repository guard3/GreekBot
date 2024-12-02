#ifndef DISCORD_MESSAGE_H
#define DISCORD_MESSAGE_H
#include "Attachment.h"
#include "Component.h"
#include "Embed.h"
#include "Member.h"
#include "MessageFwd.h"
#include "User.h"
#include <span>

enum eMessageType {
	MESSAGE_TYPE_DEFAULT,
	MESSAGE_TYPE_RECIPIENT_ADD,
	MESSAGE_TYPE_RECIPIENT_REMOVE,
	MESSAGE_TYPE_CALL,
	MESSAGE_TYPE_CHANNEL_NAME_CHANGE,
	MESSAGE_TYPE_CHANNEL_ICON_CHANGE,
	MESSAGE_TYPE_CHANNEL_PINNED_MESSAGE,
	MESSAGE_TYPE_GUILD_MEMBER_JOIN,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_1,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_2,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_3,
	MESSAGE_TYPE_CHANNEL_FOLLOW_ADD,
	// 13
	MESSAGE_TYPE_GUILD_DISCOVERY_DISQUALIFIED = 14,
	MESSAGE_TYPE_GUILD_DISCOVERY_REQUALIFIED,
	MESSAGE_TYPE_GUILD_DISCOVERY_GRACE_PERIOD_INITIAL_WARNING,
	MESSAGE_TYPE_GUILD_DISCOVERY_GRACE_PERIOD_FINAL_WARNING,
	MESSAGE_TYPE_THREAD_CREATED,
	MESSAGE_TYPE_REPLY,
	MESSAGE_TYPE_CHAT_INPUT_COMMAND,
	MESSAGE_TYPE_THREAD_STARTER_MESSAGE,
	MESSAGE_TYPE_GUILD_INVITE_REMINDER,
	MESSAGE_TYPE_CONTEXT_MENU_COMMAND
};
eMessageType tag_invoke(boost::json::value_to_tag<eMessageType>, const boost::json::value&);

enum eMessageFlag {
	MESSAGE_FLAG_NONE                                   = 0,
	MESSAGE_FLAG_CROSSPOSTED                            = 1 << 0,  // this message has been published to subscribed channels (via Channel Following)
	MESSAGE_FLAG_IS_CROSSPOST                           = 1 << 1,  // this message originated from a message in another channel (via Channel Following)
	MESSAGE_FLAG_SUPPRESS_EMBEDS                        = 1 << 2,  // do not include any embeds when serializing this message
	MESSAGE_FLAG_SOURCE_MESSAGE_DELETED                 = 1 << 3,  // the source message for this crosspost has been deleted (via Channel Following)
	MESSAGE_FLAG_URGENT                                 = 1 << 4,  // this message came from the urgent message system
	MESSAGE_FLAG_HAS_THREAD                             = 1 << 5,  // this message has an associated thread, with the same id as the message
	MESSAGE_FLAG_EPHEMERAL                              = 1 << 6,  // this message is only visible to the user who invoked the Interaction
	MESSAGE_FLAG_LOADING                                = 1 << 7,  // this message is an Interaction Response and the bot is "thinking"
	MESSAGE_FLAG_FAILED_TO_MENTION_SOME_ROLES_IN_THREAD = 1 << 8,  // this message failed to mention some roles and add their members to the thread
	MESSAGE_FLAG_SUPPRESS_NOTIFICATIONS                 = 1 << 12, // this message will not trigger push and desktop notifications
	MESSAGE_FLAG_IS_VOICE_MESSAGE                       = 1 << 13, // this message is a voice message
	/* Custom flags */
	MESSAGE_FLAG_TTS              = 1 << 15, // this is a TTS message
	MESSAGE_FLAG_MENTION_EVERYONE = 1 << 16  // this message mentions everyone
};
inline constexpr eMessageFlag operator|(eMessageFlag lhs, eMessageFlag rhs) { return static_cast<eMessageFlag>(std::to_underlying(lhs) | std::to_underlying(rhs)); }
inline constexpr eMessageFlag operator&(eMessageFlag lhs, eMessageFlag rhs) { return static_cast<eMessageFlag>(std::to_underlying(lhs) & std::to_underlying(rhs)); }

class cMessageUpdate final {
	cSnowflake m_id;
	cSnowflake m_channel_id;
	std::optional<std::string> m_content;
	std::optional<std::vector<cActionRow>> m_components;
	std::optional<std::vector<cEmbed>> m_embeds;
public:
	cMessageUpdate() = default;
	explicit cMessageUpdate(const boost::json::object&);
	explicit cMessageUpdate(const boost::json::value&);

	auto&&        GetId(this auto&& self) noexcept { return std::forward<decltype(self)>(self).m_id;         }
	auto&& GetChannelId(this auto&& self) noexcept { return std::forward<decltype(self)>(self).m_channel_id; }
	auto     GetContent(this auto&  self) noexcept { return cPtr(self.m_content    ? self.m_content.operator->()    : nullptr); }
	auto  GetComponents(this auto&  self) noexcept { return cPtr(self.m_components ? self.m_components.operator->() : nullptr); }
	auto      GetEmbeds(this auto&  self) noexcept { return cPtr(self.m_embeds     ? self.m_embeds.operator->()     : nullptr); }
	/* Resetters */
	void    ResetContent() noexcept { m_content.reset();    }
	void ResetComponents() noexcept { m_components.reset(); }
	void     ResetEmbeds() noexcept { m_embeds.reset();	    }
	/* Setters */
	template<typename Self, typename T = decltype(m_content)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	Self&& SetContent(this Self&& self, Arg&& arg) {
		self.m_content.emplace(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<typename Self, typename T = decltype(m_components)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	Self&& SetComponents(this Self&& self, Arg&& arg) {
		self.m_components.emplace(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<typename Self, typename T = decltype(m_embeds)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	Self&& SetEmbeds(this Self&& self, Arg&& arg) {
		self.m_embeds.emplace(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	/* Emplacers */
	template<typename T = decltype(m_embeds)::value_type, typename... Args> requires std::constructible_from<T, Args&&...>
	T& EmplaceContent(Args&&... args) {
		return m_content.emplace(std::forward<Args>(args)...);
	}
	template<typename T = decltype(m_components)::value_type, typename... Args> requires std::constructible_from<T, Args&&...>
	T& EmplaceComponents(Args&&... args) {
		return m_components.emplace(std::forward<Args>(args)...);
	}
	template<typename T = decltype(m_embeds)::value_type, typename... Args> requires std::constructible_from<T, Args&&...>
	T& EmplaceEmbeds(Args&&... args) {
		return m_embeds.emplace(std::forward<Args>(args)...);
	}
};
/* ========== JSON conversion ======================================================================================= */
cMessageUpdate
tag_invoke(boost::json::value_to_tag<cMessageUpdate>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cMessageUpdate&);
/* ========== Base class of cMessage and cPartialMessage ============================================================ */
class cMessageBase {
protected:
	eMessageFlag            m_flags;
	std::string             m_content;
	std::vector<cActionRow> m_components;
	std::vector<cEmbed>     m_embeds;

	explicit cMessageBase(const boost::json::value&);
	explicit cMessageBase(const boost::json::object&);
	cMessageBase() noexcept : m_flags{ MESSAGE_FLAG_NONE } {}
public:
	/* Getters */
	eMessageFlag       GetFlags() const noexcept { return m_flags;   }
	std::string_view GetContent() const noexcept { return m_content; }
	auto GetComponents(this auto& self) noexcept { return std::span(self.m_components); }
	auto     GetEmbeds(this auto& self) noexcept { return std::span(self.m_embeds);     }
	/* Movers */
	auto    MoveContent() noexcept { return std::move(m_content);    }
	auto MoveComponents() noexcept { return std::move(m_components); }
	auto     MoveEmbeds() noexcept { return std::move(m_embeds);     }
	/* Resetters */
	void      ResetFlags() noexcept { m_flags = MESSAGE_FLAG_NONE; }
	void    ResetContent() noexcept { m_content.clear();           }
	void ResetComponents() noexcept { m_components.clear();        }
	void     ResetEmbeds() noexcept { m_embeds.clear();            }
	/* Emplacers */
	eMessageFlag& EmplaceFlags(eMessageFlag flags = MESSAGE_FLAG_NONE) noexcept {
		return m_flags = flags;
	}
	std::string& EmplaceContent() noexcept {
		m_content.clear();
		return m_content;
	}
	std::vector<cActionRow>& EmplaceComponents() noexcept {
		m_components.clear();
		return m_components;
	}
	std::vector<cEmbed>& EmplaceEmbeds() noexcept {
		m_embeds.clear();
		return m_embeds;
	}
	template<typename T = decltype(m_content), typename Arg = T, typename... Args> requires std::constructible_from<T, Arg&&, Args&&...>
	T& EmplaceContent(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<T&, Arg&&> && sizeof...(Args) == 0)
			return m_content = std::forward<Arg>(arg);
		else
			return m_content = T(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename T = decltype(m_components), typename Arg = T, typename... Args> requires std::constructible_from<T, Arg&&, Args&&...>
	T& EmplaceComponents(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<T&, Arg&&> && sizeof...(Args) == 0)
			return m_components = std::forward<Arg>(arg);
		else
			return m_components = T(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename T = decltype(m_embeds), typename Arg = T, typename... Args> requires std::constructible_from<T, Arg&&, Args&&...>
	T& EmplaceEmbeds(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<T&, Arg&&> && sizeof...(Args) == 0)
			return m_embeds = std::forward<Arg>(arg);
		else
			return m_embeds = T(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	/* Setters */
	template<typename Self>
	Self&& SetFlags(this Self&& self, eMessageFlag flags) noexcept {
		self.m_flags = flags;
		return std::forward<Self>(self);
	}
	template<typename Self, typename T = decltype(m_content), typename Arg = T> requires std::constructible_from<T, Arg&&>
	Self&& SetContent(this Self&& self, Arg&& arg) {
		self.EmplaceContent(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<typename Self, typename T = decltype(m_components), typename Arg = T> requires std::constructible_from<T, Arg&&>
	Self&& SetComponents(this Self&& self, Arg&& arg) {
		self.EmplaceComponents(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<typename Self, typename T = decltype(m_embeds), typename Arg = T> requires std::constructible_from<T, Arg&&>
	Self&& SetEmbeds(this Self&& self, Arg&& arg) {
		self.EmplaceEmbeds(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
};
/* ========== JSON conversion ======================================================================================= */
void
tag_invoke(boost::json::value_from_tag, boost::json::value& v, const cMessageBase&);
/* ========== A partial message, used for sending new messages ====================================================== */
class cPartialMessage final : public cMessageBase {};
/* ========== The main message class ================================================================================ */
class cMessage final : public cMessageBase {
	cSnowflake  id;
	cSnowflake  channel_id;
	cUser       author;
	std::chrono::sys_time<std::chrono::milliseconds> timestamp;
	std::chrono::sys_time<std::chrono::milliseconds> edited_timestamp;
	// ...
	eMessageType type;

	std::vector<cAttachment> attachments;

public:
	explicit cMessage(const boost::json::object&);
	explicit cMessage(const boost::json::value&);
	/* Getters */
	auto            GetType() const noexcept { return type;             }
	auto       GetTimestamp() const noexcept { return timestamp;        }
	auto GetEditedTimestamp() const noexcept { return edited_timestamp; }
	auto&&        GetId(this auto&& self) noexcept { return std::forward<decltype(self)>(self).id;         }
	auto&& GetChannelId(this auto&& self) noexcept { return std::forward<decltype(self)>(self).channel_id; }
	auto&&    GetAuthor(this auto&& self) noexcept { return std::forward<decltype(self)>(self).author;     }
	auto GetAttachments(this auto&  self) noexcept { return std::span(self.attachments);                   }
	/* Movers */
	auto      MoveAuthor() noexcept { return std::move(author);      }
	auto MoveAttachments() noexcept { return std::move(attachments); }
};
/* ========== JSON conversion ======================================================================================= */
cMessage
tag_invoke(boost::json::value_to_tag<cMessage>, const boost::json::value&);
/* ================================================================================================================== */
inline crefMessage::crefMessage(const cMessage &msg) noexcept : m_id(msg.GetId()) {}
#endif /* DISCORD_MESSAGE_H */