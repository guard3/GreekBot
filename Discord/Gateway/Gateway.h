#ifndef GREEKBOT_GATEWAY_H
#define GREEKBOT_GATEWAY_H
#include "Common.h"
#include "Coroutines.h"
#include "User.h"
#include "Guild.h"
#include "Interaction.h"

enum eIntent {
	INTENT_GUILDS                    = 1 << 0,
	INTENT_GUILD_MEMBERS             = 1 << 1,
	INTENT_GUILD_BANS                = 1 << 2,
	INTENT_GUILD_EMOJIS_AND_STICKERS = 1 << 3,
	INTENT_GUILD_INTEGRATIONS        = 1 << 4,
	INTENT_GUILD_WEBHOOKS            = 1 << 5,
	INTENT_GUILD_INVITES             = 1 << 6,
	INTENT_GUILD_VOICE_STATES        = 1 << 7,
	INTENT_GUILD_PRESENCES           = 1 << 8,
	INTENT_GUILD_MESSAGES            = 1 << 9,
	INTENT_GUILD_MESSAGE_REACTIONS   = 1 << 10,
	INTENT_GUILD_MESSAGE_TYPING      = 1 << 11,
	INTENT_DIRECT_MESSAGES           = 1 << 12,
	INTENT_DIRECT_MESSAGE_REACTIONS  = 1 << 13,
	INTENT_DIRECT_MESSAGE_TYPING     = 1 << 14
};
inline eIntent operator|(eIntent a, eIntent b) { return (eIntent)((int)a | (int)b); }
inline eIntent operator&(eIntent a, eIntent b) { return (eIntent)((int)a & (int)b); }

class cHttpField final {
private:
	std::string m_name, m_value;
public:
	cHttpField(std::string n, std::string v) : m_name(std::move(n)), m_value(std::move(v)) {}
	const std::string& GetName()  const noexcept { return m_name;  }
	const std::string& GetValue() const noexcept { return m_value; }
};
typedef std::vector<cHttpField> tHttpFields;

KW_DECLARE(query, KW_QUERY, std::string)
KW_DECLARE(user_ids, KW_USER_IDS, std::vector<cSnowflake>)

class cGateway {
private:
	class implementation;
	uhHandle<implementation> m_pImpl;

	static inline tHttpFields m_empty;

	cAsyncGenerator<cMember> get_guild_members(const cSnowflake&, const std::string&, const std::vector<cSnowflake>&);

protected:
	const char* GetHttpAuthorization() const noexcept;

public:
	cGateway(const char* token, eIntent intents);
	cGateway(const cGateway&) = delete;
	~cGateway();

	cGateway& operator=(const cGateway&) = delete;

	const char* GetToken() const noexcept;

	cTask<json::value> DiscordGet   (const std::string& path,                          const tHttpFields& fields = m_empty);
	cTask<json::value> DiscordPost  (const std::string& path, const json::object& obj, const tHttpFields& fields = m_empty);
	cTask<json::value> DiscordPatch (const std::string& path, const json::object& obj, const tHttpFields& fields = m_empty);
	cTask<json::value> DiscordPut   (const std::string& path,                          const tHttpFields& fields = m_empty);
	cTask<json::value> DiscordPut   (const std::string& path, const json::object& obj, const tHttpFields& fields = m_empty);
	cTask<json::value> DiscordDelete(const std::string& path,                          const tHttpFields& fields = m_empty);

	cTask<json::value> DiscordPostNoRetry(const std::string& path, const json::object& obj, const tHttpFields& fields = m_empty);

	cTask<> ResumeOnEventThread();
	cTask<> WaitOnEventThread(std::chrono::milliseconds);
	cAsyncGenerator<cMember> GetGuildMembers(const cSnowflake& guild_id, iKwArg auto&... kwargs) {
		cKwPack pack{ kwargs... };
		return get_guild_members(guild_id, KwGet<KW_QUERY>(pack), KwGet<KW_USER_IDS>(pack));
	}

	virtual cTask<> OnReady(uhUser) { co_return; }
	virtual cTask<> OnUserUpdate(uhUser) { co_return; }
	virtual cTask<> OnGuildCreate(uhGuild) { co_return; }
	virtual cTask<> OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) { co_return; }
	virtual cTask<> OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) { co_return; }
	virtual cTask<> OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) { co_return; }
	virtual cTask<> OnInteractionCreate(const cInteraction&) { co_return; }
	virtual cTask<> OnMessageCreate(const cMessage&) { co_return; }

	void Run();
};
#endif /* GREEKBOT_GATEWAY_H */