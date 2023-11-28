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
#include "Modal.h"

// TODO: better error messages
class xInteractionError : public std::logic_error {
protected:
	xInteractionError(const char* msg) : std::logic_error(msg) {}
};
class xInteractionRespondedError : public xInteractionError {
public:
	xInteractionRespondedError() : xInteractionError("Interaction already responded to") {}
};
class xInteractionCallbackError : public xInteractionError {
public:
	xInteractionCallbackError() : xInteractionError("This callback is not supported") {}
};

class cBot : public cGateway {
private:
	uhUser m_user;

	cTask<> OnReady(uhUser) override;
	cTask<> OnUserUpdate(uhUser) override;

	cTask<cMessage> create_message(const cSnowflake& channel_id, const cMessageParams&);
	cTask<cMessage> edit_message(const cSnowflake&, const cSnowflake&, const cMessageParams&);
	cTask<> modify_guild_member(const cSnowflake&, const cSnowflake&, const cMemberOptions&);

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
	cTask<> AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	cTask<> RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	cTask<> UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids);
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
	/* TODO: implement these */
	cTask<> InteractionEditMessage();
	cTask<> InteractionGetMessage();
	cTask<> InteractionDeleteMessage();
	/* Current interaction functions */
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> RespondToInteraction(const cAppCmdInteraction&, bool bThinking = false);
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> RespondToInteraction(const cMsgCompInteraction&, bool bThinking = false);
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> RespondToInteraction(const cModalSubmitInteraction&, bool bThinking = false);
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> RespondToInteraction(const cInteraction&, bool bThinking = false);
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> RespondToInteraction(const cInteraction& i, const cMessageParams& p);
	template<kw::key... Keys> requires (sizeof...(Keys) > 0)
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> RespondToInteraction(const cInteraction& i, kw::arg<Keys>&... kwargs) {
		co_await RespondToInteraction(i, cMessageParams{ kwargs... });
	}
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> RespondToInteractionWithModal(const cInteraction&, const cModal&);

	cTask<> EditInteractionResponse(const cInteraction&, const cMessageParams&);
	template<kw::key... Keys>
	cTask<> EditInteractionResponse(const cInteraction& i, kw::arg<Keys>&... kwargs) {
		co_await EditInteractionResponse(i, cMessageParams{ kwargs... });
	}
	cTask<> DeleteInteractionResponse(const cInteraction&);
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> SendInteractionFollowupMessage(const cInteraction&, const cMessageParams&);
	template<kw::key... Keys>
	[[deprecated("Use InteractionXXX variants")]]
	cTask<> SendInteractionFollowupMessage(const cInteraction& i, kw::arg<Keys>&... kwargs) {
		co_await SendInteractionFollowupMessage(i, cMessageParams{ kwargs... });
	}

	cTask<int> BeginGuildPrune(const cSnowflake& id, int days, std::string_view reason = {});

	cTask<cChannel> CreateDM(const cSnowflake& recipient_id);
	template<kw::key... Keys>
	cTask<cMessage> CreateMessage(const cSnowflake& channel_id, kw::arg<Keys>&... kwargs) {
		co_return co_await create_message(channel_id, cMessageParams{ kwargs... });
	}
	template<kw::key... Keys>
	cTask<cMessage> CreateDMMessage(const cSnowflake& recipient_id, kw::arg<Keys>&... kwargs) {
		co_return co_await CreateMessage((co_await CreateDM(recipient_id)).GetId(), kwargs...);
	}
	template<kw::key... Keys>
	cTask<cMessage> EditMessage(const cSnowflake& channel_id, const cSnowflake& msg_id, kw::arg<Keys>&... kwargs) {
		co_return co_await edit_message(channel_id, msg_id, cMessageParams{ kwargs... });
	}
	cTask<> DeleteMessage(const cSnowflake& channel_id, const cSnowflake& msg_id, std::string_view reason = {});
	cTask<cMessage> GetChannelMessage(const cSnowflake& channel_id, const cSnowflake& message_id);

	template<kw::key... Keys>
	cTask<> ModifyGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, kw::arg<Keys>&... kwargs) {
		co_await modify_guild_member(guild_id, user_id, { kwargs... });
	}
	cTask<> RemoveGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason = {});
	cTask<> CreateGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::chrono::seconds delete_message_seconds = std::chrono::seconds(0), std::string_view reason = {});
	cTask<> RemoveGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason = {});
};
#endif // GREEKBOT_BOT_H