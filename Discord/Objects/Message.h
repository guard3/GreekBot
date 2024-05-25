#ifndef GREEKBOT_MESSAGE_H
#define GREEKBOT_MESSAGE_H
#include "User.h"
#include "Embed.h"
#include "Member.h"
#include "Component.h"
#include "Attachment.h"
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
	MESSAGE_FLAG_CROSSPOSTED                            = 1 << 0, // this message has been published to subscribed channels (via Channel Following)
	MESSAGE_FLAG_IS_CROSSPOST                           = 1 << 1, // this message originated from a message in another channel (via Channel Following)
	MESSAGE_FLAG_SUPPRESS_EMBEDS                        = 1 << 2, // do not include any embeds when serializing this message
	MESSAGE_FLAG_SOURCE_MESSAGE_DELETED                 = 1 << 3, // the source message for this crosspost has been deleted (via Channel Following)
	MESSAGE_FLAG_URGENT                                 = 1 << 4, // this message came from the urgent message system
	MESSAGE_FLAG_HAS_THREAD                             = 1 << 5, // this message has an associated thread, with the same id as the message
	MESSAGE_FLAG_EPHEMERAL                              = 1 << 6, // this message is only visible to the user who invoked the Interaction
	MESSAGE_FLAG_LOADING                                = 1 << 7, // this message is an Interaction Response and the bot is "thinking"
	MESSAGE_FLAG_FAILED_TO_MENTION_SOME_ROLES_IN_THREAD = 1 << 8, // this message failed to mention some roles and add their members to the thread

	/* Custom flags */
	MESSAGE_FLAG_TTS              = 1 << 15, // this is a TTS message
	MESSAGE_FLAG_MENTION_EVERYONE = 1 << 16  // this message mentions everyone
};
inline eMessageFlag operator|(eMessageFlag a, eMessageFlag b) { return (eMessageFlag)((int)a | (int)b); }
inline eMessageFlag operator&(eMessageFlag a, eMessageFlag b) { return (eMessageFlag)((int)a & (int)b); }

KW_DECLARE(flags, eMessageFlag)
KW_DECLARE(content, std::string)

/* TODO: make cMessage inherit from here */
/* TODO: maybe also make a base discord object class? */
/* Base class of cMessage; used for creating or editing messages */
class cMessageParams {
private:
	eMessageFlag m_flags;
	std::optional<std::string> m_content;
	std::optional<std::vector<cActionRow>> m_components;
	std::optional<std::vector<cEmbed>> m_embeds;

public:
	cMessageParams() noexcept : m_flags{ MESSAGE_FLAG_NONE } {}

	void ResetFlags() noexcept {
		m_flags = MESSAGE_FLAG_NONE;
	}
	void ResetContent() noexcept {
		m_content.reset();
	}
	void ResetComponents() noexcept {
		m_components.reset();
	}
	void ResetEmbeds() noexcept {
		m_embeds.reset();
	}

	cMessageParams& SetFlags(eMessageFlag flags) noexcept {
		m_flags = flags;
		return *this;
	}
	template<typename T = std::string> requires std::constructible_from<std::string, T&&>
	cMessageParams& SetContent(T&& arg) {
		m_content.emplace(std::forward<T>(arg));
		return *this;
	}
	template<typename T = std::vector<cActionRow>> requires std::constructible_from<std::vector<cActionRow>, T&&>
	cMessageParams& SetComponents(T&& arg) {
		m_components.emplace(std::forward<T>(arg));
		return *this;
	}
	template<typename T = std::vector<cEmbed>> requires std::constructible_from<std::vector<cEmbed>, T&&>
	cMessageParams& SetEmbeds(T&& arg) {
		m_embeds.emplace(std::forward<T>(arg));
		return *this;
	}

	eMessageFlag& EmplaceFlags(eMessageFlag flags) noexcept {
		return m_flags = flags;
	}
	template<typename... Args> requires std::constructible_from<std::string, Args&&...>
	std::string& EmplaceContent(Args&&... args) {
		return m_content.emplace(std::forward<Args>(args)...);
	}
	template<typename... Args> requires std::constructible_from<std::vector<cActionRow>, Args&&...>
	std::vector<cActionRow>& EmplaceComponents(Args&&... args) {
		return m_components.emplace(std::forward<Args>(args)...);
	}
	template<typename... Args> requires std::constructible_from<std::vector<cEmbed>, Args&&...>
	std::vector<cEmbed>& EmplaceEmbeds(Args&&... args) {
		return m_embeds.emplace(std::forward<Args>(args)...);
	}

	friend void tag_invoke(const json::value_from_tag&, json::value& v, const cMessageParams&);
};

void tag_invoke(const json::value_from_tag&, json::value& v, const cMessageParams&);

class cMessageUpdate final {
private:
	cSnowflake m_id;
	cSnowflake m_channel_id;
	std::optional<std::string> m_content;
	std::optional<std::vector<cActionRow>> m_components;
	std::optional<std::vector<cEmbed>> m_embeds;
public:
	cMessageUpdate() = default;
	explicit cMessageUpdate(const json::object&);
	explicit cMessageUpdate(const json::value&);

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

void
tag_invoke(json::value_from_tag, json::value&, const cMessageUpdate&);

class cMessage final {
private:
	cSnowflake  id;
	cSnowflake  channel_id;
	cUser       author;
	std::string content;
	std::string timestamp;
	std::string edited_timestamp;
	// ...
	eMessageType type;
	// ...
	eMessageFlag flags;

	std::vector<cEmbed> embeds;
	std::vector<cAttachment> attachments;

public:
	explicit cMessage(const json::object&);
	explicit cMessage(const json::value&);

	const cSnowflake& GetId() const noexcept { return id; }
	const cSnowflake& GetChannelId() const noexcept { return channel_id; }
	const cUser& GetAuthor() const noexcept { return author; }
	std::string_view GetContent() const noexcept { return content; }
	std::string_view GetTimestamp() const noexcept { return timestamp; }
	std::string_view GetEditedTimestamp() const noexcept { return edited_timestamp; }
	eMessageType GetType() const noexcept { return type; }
	eMessageFlag GetFlags() const noexcept { return flags; }
	std::span<const cEmbed> GetEmbeds() const noexcept { return embeds; }
	std::span<const cAttachment> GetAttachments() const noexcept { return attachments; }

	cSnowflake& GetId() noexcept { return id; }
	cSnowflake& GetChannelId() noexcept { return channel_id; }
	cUser& GetAuthor() noexcept { return author; };
	std::span<cEmbed> GetEmbeds() noexcept { return embeds; }
	std::span<cAttachment> GetAttachments() noexcept { return attachments; }

	std::string MoveContent() noexcept { return std::move(content); }
	std::string MoveTimestamp() noexcept { return std::move(timestamp); }
	std::string MoveEditedTimestamp() noexcept { return std::move(edited_timestamp); }
	std::vector<cEmbed> MoveEmbeds() noexcept { return std::move(embeds); }
	std::vector<cAttachment> MoveAttachments() noexcept { return std::move(attachments); }
};
typedef   hHandle<cMessage>   hMessage;
typedef  chHandle<cMessage>  chMessage;
typedef  uhHandle<cMessage>  uhMessage;
typedef uchHandle<cMessage> uchMessage;

class crefMessage final {
private:
	const cSnowflake& m_id;
public:
	crefMessage(const cMessage& msg) noexcept : m_id(msg.GetId()) {}
	crefMessage(const cSnowflake& id) noexcept : m_id(id) {}

	const cSnowflake& GetId() const noexcept { return m_id; }
};
#endif /* GREEKBOT_MESSAGE_H */