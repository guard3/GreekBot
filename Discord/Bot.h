#ifndef GREEKBOT_BOT_H
#define GREEKBOT_BOT_H
#include "Interaction.h"
#include "Gateway.h"
#include "Message.h"
#include "Embed.h"
#include "Role.h"
#include <vector>
#include "User.h"
#include "Guild.h"
#include "Channel.h"

class cBot : public cGateway {
private:
	uhUser m_user;

	cTask<> OnReady(uhUser) override;

protected:
	using cGateway::OnInteractionCreate;
	using cGateway::OnGuildCreate;
	using cGateway::OnGuildRoleCreate;
	using cGateway::OnGuildRoleUpdate;
	using cGateway::OnGuildRoleDelete;
	using cGateway::OnMessageCreate;
	
public:
	explicit cBot(const char* token, eIntent intents) : cGateway(token, intents) {}

	using cGateway::GetToken;

	chUser GetUser() const { return m_user.get(); }

	cTask<cUser> GetUser(const cSnowflake& user_id);
	cTask<cMember> GetGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id);
	cTask<std::vector<cRole>> GetGuildRoles(const cSnowflake& guild_id);
	cTask<> AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	cTask<> RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	cTask<> UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids);

	cTask<> AcknowledgeInteraction(const cInteraction& interaction);
	cTask<> RespondToInteraction(const cInteraction&, const cMessageParams&);
	template<iKwArg... KwArgs>
	cTask<> RespondToInteraction(const cInteraction& i, KwArgs&... kwargs) {
		return RespondToInteraction(i, { kwargs... });
	}
	cTask<> EditInteractionResponse(const cInteraction&, const cMessageParams&);
	template<iKwArg... KwArgs>
	cTask<> EditInteractionResponse(const cInteraction& i, KwArgs&... kwargs) {
		return EditInteractionResponse(i, { kwargs... });
	}
	cTask<> SendInteractionFollowupMessage(const cInteraction& i, const cMessageParams&);
	template<iKwArg... KwArgs>
	cTask<> SendInteractionFollowupMessage(const cInteraction& i, KwArgs&... kwargs) {
		return SendInteractionFollowupMessage(i, { kwargs... });
	}

	cTask<int> BeginGuildPrune(const cSnowflake& id, int days, std::string reason = {});

	cTask<cChannel> CreateDM(const cSnowflake& recipient_id);
	cTask<cMessage> CreateMessage(const cSnowflake& channel_id, const cMessageParams&);
	cTask<cMessage> CreateDMMessage(const cSnowflake& recipient_id, const cMessageParams&);
	template<iKwArg... KwArgs>
	cTask<cMessage> CreateMessage(const cSnowflake& channel_id, KwArgs&... kwargs) {
		return CreateMessage(channel_id, { kwargs... });
	}
	template<iKwArg... KwArgs>
	cTask<cMessage> CreateDMMessage(const cSnowflake& recipient_id, KwArgs&... kwargs) {
		co_return co_await CreateDMMessage(recipient_id, { kwargs... });
	}

	cTask<> CreateGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, chrono::seconds delete_message_seconds = chrono::seconds(0), const std::string& reason = {});
	cTask<> RemoveGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, const std::string& reason = {});
};
#endif // GREEKBOT_BOT_H