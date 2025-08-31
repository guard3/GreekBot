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
#include <optional>

class cBot : public cGateway {
private:
	std::optional<cUser> m_user;

	cTask<> OnReady(cUser&) override;
	cTask<> OnUserUpdate(cUser&) override;

	cAsyncGenerator<cMessage> get_channel_messages(std::string);
	cTask<> delete_message(const cSnowflake& channel_id, const cSnowflake& msg_id, std::span<const cHttpField> fields);

	cTask<cMessage> InteractionEditMessageImpl(const cInteraction&, const cMessageUpdate&, crefMessage);

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

	const cUser& GetUser() const noexcept { return m_user.value(); }
	cUser& GetUser() noexcept { return m_user.value(); }

	cTask<cUser> GetUser(crefUser user);
	cTask<cMember> GetGuildMember(crefGuild guild, crefUser user);
	cTask<std::vector<cRole>> GetGuildRoles(crefGuild guild);
	cTask<> AddGuildMemberRole(crefGuild guild, crefUser user, crefRole role);
	cTask<> RemoveGuildMemberRole(crefGuild guild_id, crefUser user, crefRole role);
	/* Interactions - Defer message or update */
	cTask<> InteractionDefer(const cAppCmdInteraction&, bool thinking = false);
	cTask<> InteractionDefer(const cMsgCompInteraction&, bool thinking = false);
	cTask<> InteractionDefer(const cModalSubmitInteraction&, bool thinking = false);
	cTask<> InteractionDefer(const cInteraction&, bool thinking = false);
	/* Interactions - Send new message */
	cTask<> InteractionSendMessage(const cInteraction&, const cMessageBase&);
	/* Interactions - Send modal */
	cTask<> InteractionSendModal(const cAppCmdInteraction&, const cModal&);
	cTask<> InteractionSendModal(const cMsgCompInteraction&, const cModal&);
	cTask<> InteractionSendModal(const cInteraction&, const cModal&);
	/* Interactions - Extra functions */
	cTask<cMessage> InteractionEditMessage(const cMsgCompInteraction&, const cMessageUpdate&, crefMessage = cSnowflake());
	cTask<cMessage> InteractionEditMessage(const cInteraction&, const cMessageUpdate&, crefMessage = cSnowflake());
	cTask<cMessage> InteractionGetMessage(const cInteraction&, crefMessage = cSnowflake());
	cTask<>         InteractionDeleteMessage(const cInteraction&, crefMessage = cSnowflake());

	cTask<int> BeginGuildPrune(crefGuild guild, int days, std::string_view reason = {});

	cTask<cChannel> CreateDM(crefUser recipient);
	cTask<cMessage> CreateMessage(crefChannel channel, const cMessageBase& msg);
	cTask<cMessage> CreateDMMessage(crefUser recipient, const cMessageBase& msg);
	cTask<cMessage> EditMessage(crefChannel channel, crefMessage target_msg, const cMessageUpdate& msg);
	cTask<> DeleteMessage(crefChannel channel, crefMessage msg, std::string_view reason = {});
	cTask<> DeleteMessages(crefChannel channel, std::span<const cSnowflake> msg_ids, std::string_view reason = {});
	cTask<cMessage> GetChannelMessage(crefChannel channel, crefMessage message);
	cAsyncGenerator<cMessage> GetChannelMessages(crefChannel channel, std::size_t limit = 50);
	cAsyncGenerator<cMessage> GetChannelMessagesBefore(crefChannel channel, crefMessage before_this_message, std::size_t limit = 50);

	cTask<> ModifyGuildMember(crefGuild guild, crefUser user, const cMemberOptions&);
	cTask<> RemoveGuildMember(crefGuild guild, crefUser user, std::string_view reason = {});
	cTask<> CreateGuildBan(crefGuild guild, crefUser user, std::chrono::seconds delete_message_seconds = std::chrono::seconds(0), std::string_view reason = {});
	cTask<> RemoveGuildBan(crefGuild guild, crefUser user, std::string_view reason = {});
};
#endif // GREEKBOT_BOT_H