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
	cTask<> OnUserUpdate(uhUser) override;

	cTask<> respond_to_interaction(const cInteraction&, const cMessageParams&);
	cTask<> edit_interaction_response(const cInteraction&, const cMessageParams&);
	cTask<> send_interaction_followup_message(const cInteraction& i, const cMessageParams&);
	cTask<cMessage> create_message(const cSnowflake& channel_id, const cMessageParams&);

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

	cTask<> RespondToInteraction(const cInteraction& i, iKwArg auto&... kwargs) {
		co_await respond_to_interaction(i, { kwargs... });
	}
	template<>
	cTask<> RespondToInteraction<>(const cInteraction&);
	cTask<> EditInteractionResponse(const cInteraction& i, iKwArg auto&... kwargs) {
		co_await edit_interaction_response(i, { kwargs... });
	}
	cTask<> DeleteInteractionResponse(const cInteraction&);
	cTask<> SendInteractionFollowupMessage(const cInteraction& i, iKwArg auto&... kwargs) {
		co_await send_interaction_followup_message(i, { kwargs... });
	}

	cTask<int> BeginGuildPrune(const cSnowflake& id, int days, const std::string& reason = {});

	cTask<cChannel> CreateDM(const cSnowflake& recipient_id);
	cTask<cMessage> CreateMessage(const cSnowflake& channel_id, iKwArg auto&... kwargs) {
		co_return co_await create_message(channel_id, { kwargs... });
	}
	cTask<cMessage> CreateDMMessage(const cSnowflake& recipient_id, iKwArg auto&... kwargs) {
		co_return co_await CreateMessage((co_await CreateDM(recipient_id)).GetId(), kwargs...);
	}

	cTask<> CreateGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, chrono::seconds delete_message_seconds = 0s, const std::string& reason = {});
	cTask<> RemoveGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, const std::string& reason = {});
};
#endif // GREEKBOT_BOT_H