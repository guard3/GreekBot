#ifndef GREEKBOT_BOT_H
#define GREEKBOT_BOT_H
#include "Exception.h"
#include "Interaction.h"
#include "Gateway.h"
#include "Message.h"
#include "Embed.h"
#include "Role.h"
#include <vector>
#include "User.h"
#include "Guild.h"
#include "Channel.h"
#include "Modal.h"

class cBot : public cGateway {
private:
	uhUser m_user;

	cTask<> OnReady(uhUser) override;
	cTask<> OnUserUpdate(uhUser) override;

protected:
	using cGateway::OnInteractionCreate;
	using cGateway::OnGuildCreate;
	using cGateway::OnGuildRoleCreate;
	using cGateway::OnGuildRoleUpdate;
	using cGateway::OnGuildRoleDelete;
	using cGateway::OnMessageCreate;
	
public:
	explicit cBot(std::string_view token, eIntent intents) : cGateway(token, intents) {}

	using cGateway::GetToken;

	chUser GetUser() const { return m_user.get(); }

	cTask<cUser> GetUser(const cSnowflake& user_id);
	cTask<cMember> GetGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id);
	cTask<std::vector<cRole>> GetGuildRoles(const cSnowflake& guild_id);
	cTask<> AddGuildMemberRole(const cSnowflake& guild_id, crefUser user, const cSnowflake& role_id);
	cTask<> RemoveGuildMemberRole(const cSnowflake& guild_id, crefUser user, const cSnowflake& role_id);
	/* Interactions - Defer message or update */
	cTask<> InteractionDefer(const cAppCmdInteraction&, bool thinking = false);
	cTask<> InteractionDefer(const cMsgCompInteraction&, bool thinking = false);
	cTask<> InteractionDefer(const cModalSubmitInteraction&, bool thinking = false);
	cTask<> InteractionDefer(const cInteraction&, bool thinking = false);
	/* Interactions - Send new message */
	cTask<> InteractionSendMessage(const cInteraction&, const cMessageParams&);
	/* Interactions - Send modal */
	cTask<> InteractionSendModal(const cAppCmdInteraction&, const cModal&);
	cTask<> InteractionSendModal(const cMsgCompInteraction&, const cModal&);
	cTask<> InteractionSendModal(const cInteraction&, const cModal&);
	/* Interactions - Extra functions */
	cTask<cMessage> InteractionEditMessage(const cInteraction&, const cMessageUpdate&, crefMessage = cSnowflake());
	cTask<cMessage> InteractionGetMessage(const cInteraction&, crefMessage = cSnowflake());
	cTask<>         InteractionDeleteMessage(const cInteraction&, crefMessage = cSnowflake());

	cTask<int> BeginGuildPrune(const cSnowflake& id, int days, std::string_view reason = {});

	cTask<cChannel> CreateDM(const cSnowflake& recipient_id);
	cTask<cMessage> CreateMessage(crefChannel channel, const cMessageParams& msg);
	cTask<cMessage> CreateDMMessage(const cSnowflake& recipient_id, const cMessageParams& msg);
	cTask<cMessage> EditMessage(const cSnowflake& channel_id, const cSnowflake& target_msg, const cMessageUpdate& msg);
	cTask<> DeleteMessage(const cSnowflake& channel_id, const cSnowflake& msg_id, std::string_view reason = {});
	cTask<cMessage> GetChannelMessage(const cSnowflake& channel_id, const cSnowflake& message_id);

	cTask<> ModifyGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, const cMemberOptions&);
	cTask<> RemoveGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason = {});
	cTask<> CreateGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::chrono::seconds delete_message_seconds = std::chrono::seconds(0), std::string_view reason = {});
	cTask<> RemoveGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason = {});
};
#endif // GREEKBOT_BOT_H