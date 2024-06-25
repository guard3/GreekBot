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

enum eMessageFlag : std::uint32_t {
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
inline eMessageFlag operator|(eMessageFlag a, eMessageFlag b) { return (eMessageFlag)((int)a | (int)b); }
inline eMessageFlag operator&(eMessageFlag a, eMessageFlag b) { return (eMessageFlag)((int)a & (int)b); }

/* TODO: make cMessage inherit from here */
/* Base class of cMessage; used for creating messages */
class cPartialMessage {
private:
	eMessageFlag            m_flags;
	std::string             m_content;
	std::vector<cActionRow> m_components;
	std::vector<cEmbed>     m_embeds;

public:
	explicit cPartialMessage(const boost::json::value&);
	explicit cPartialMessage(const boost::json::object&);
	cPartialMessage() noexcept : m_flags{ MESSAGE_FLAG_NONE } {}
	/* Getters */
	eMessageFlag                     GetFlags() const noexcept { return m_flags;      }
	std::string_view               GetContent() const noexcept { return m_content;    }
	std::span<const cActionRow> GetComponents() const noexcept { return m_components; }
	std::span<      cActionRow> GetComponents()       noexcept { return m_components; }
	std::span<const cEmbed>         GetEmbeds() const noexcept { return m_embeds;     }
	std::span<      cEmbed>         GetEmbeds()       noexcept { return m_embeds;     }
	/* Movers */
	std::string                MoveContent() noexcept { return std::move(m_content);    }
	std::vector<cActionRow> MoveComponents() noexcept { return std::move(m_components); }
	std::vector<cEmbed>         MoveEmbeds() noexcept { return std::move(m_embeds);     }
	/* Resetters */
	void ResetFlags() noexcept {
		m_flags = MESSAGE_FLAG_NONE;
	}
	void ResetContent() noexcept {
		m_content.clear();
	}
	void ResetComponents() noexcept {
		m_components.clear();
	}
	void ResetEmbeds() noexcept {
		m_embeds.clear();
	}
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
	cPartialMessage& SetFlags(eMessageFlag flags) & noexcept { m_flags = flags; return *this; }
	template<typename T = decltype(m_content), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cPartialMessage& SetContent(Arg&& arg) & { EmplaceContent(std::forward<Arg>(arg)); return *this; }
	template<typename T = decltype(m_components), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cPartialMessage& SetComponents(Arg&& arg) & { EmplaceComponents(std::forward<Arg>(arg)); return *this; }
	template<typename T = decltype(m_embeds), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cPartialMessage& SetEmbeds(Arg&& arg) & { EmplaceEmbeds(std::forward<Arg>(arg)); return *this; }

	cPartialMessage&& SetFlags(eMessageFlag flags) && noexcept { return std::move(SetFlags(flags)); }
	template<typename T = decltype(m_content), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cPartialMessage&& SetContent(Arg&& arg) && { return std::move(SetContent(std::forward<Arg>(arg))); }
	template<typename T = decltype(m_components), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cPartialMessage&& SetComponents(Arg&& arg) && { return std::move(SetComponents(std::forward<Arg>(arg))); }
	template<typename T = decltype(m_embeds), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cPartialMessage&& SetEmbeds(Arg&& arg) && { return std::move(SetEmbeds(std::forward<Arg>(arg))); }
};

void
tag_invoke(boost::json::value_from_tag, boost::json::value& v, const cPartialMessage&);

class cMessageUpdate final {
private:
	cSnowflake m_id;
	cSnowflake m_channel_id;
	std::optional<std::string> m_content;
	std::optional<std::vector<cActionRow>> m_components;
	std::optional<std::vector<cEmbed>> m_embeds;
public:
	cMessageUpdate() = default;
	explicit cMessageUpdate(const boost::json::object&);
	explicit cMessageUpdate(const boost::json::value&);

	const cSnowflake& GetId() const noexcept { return m_id; }
	const cSnowflake& GetChannelId() const noexcept { return m_channel_id; }
	cPtr<const std::string> GetContent() const noexcept {
		return m_content ? m_content.operator->() : nullptr;
	}
	cPtr<const std::vector<cActionRow>> GetComponents() const noexcept {
		return m_components ? m_components.operator->() : nullptr;
	}
	cPtr<const std::vector<cEmbed>> GetEmbeds() const noexcept {
		return m_embeds ? m_embeds.operator->() : nullptr;
	}
	/* Resetters */
	void ResetContent() noexcept {
		m_content.reset();
	}
	void ResetComponents() noexcept {
		m_components.reset();
	}
	void ResetEmbeds() noexcept {
		m_embeds.reset();
	}
	/* Setters */
	template<typename T = decltype(m_content)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	cMessageUpdate& SetContent(Arg&& arg) {
		m_content.emplace(std::forward<Arg>(arg));
		return *this;
	}
	template<typename T = decltype(m_components)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	cMessageUpdate& SetComponents(Arg&& arg) {
		m_components.emplace(std::forward<Arg>(arg));
		return *this;
	}
	template<typename T = decltype(m_embeds)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	cMessageUpdate& SetEmbeds(Arg&& arg) {
		m_embeds.emplace(std::forward<Arg>(arg));
		return *this;
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

cMessageUpdate
tag_invoke(boost::json::value_to_tag<cMessageUpdate>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cMessageUpdate&);

class cMessage final : public cPartialMessage {
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
	const cSnowflake&        GetId() const noexcept { return id;               }
	      cSnowflake&        GetId()       noexcept { return id;               }
	const cSnowflake& GetChannelId() const noexcept { return channel_id;       }
	      cSnowflake& GetChannelId()       noexcept { return channel_id;       }
	const cUser&         GetAuthor() const noexcept { return author;           }
	      cUser&         GetAuthor()       noexcept { return author;           }
	eMessageType           GetType() const noexcept { return type;             }
	auto              GetTimestamp() const noexcept { return timestamp;        }
	auto        GetEditedTimestamp() const noexcept { return edited_timestamp; }
	std::span<const cAttachment> GetAttachments() const noexcept { return attachments; }
	std::span<      cAttachment> GetAttachments()       noexcept { return attachments; }
	/* Movers */
	cUser MoveAuthor() noexcept { return std::move(author); }
	std::vector<cAttachment> MoveAttachments() noexcept { return std::move(attachments); }
};

class crefMessage final {
private:
	const cSnowflake& m_id;
public:
	crefMessage(const cMessage& msg) noexcept : m_id(msg.GetId()) {}
	crefMessage(const cSnowflake& id) noexcept : m_id(id) {}

	const cSnowflake& GetId() const noexcept { return m_id; }
};
#endif /* DISCORD_MESSAGE_H */