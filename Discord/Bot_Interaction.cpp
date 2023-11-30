#include "Bot.h"
#include "json.h"

template<typename... Ts>
struct visitor : Ts... { using Ts::operator()...; };

template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;

enum {
	PONG                        = 1,         // ACK a Ping
	CHANNEL_MESSAGE_WITH_SOURCE = 4,         // respond to an interaction with a message
	DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE,    // ACK an interaction and edit a response later, the user sees a loading state
	DEFERRED_UPDATE_MESSAGE,                 // for components, ACK an interaction and edit the original message later; the user does not see a loading state
	UPDATE_MESSAGE,                          // for components, edit the message the component was attached to
	APPLICATION_COMMAND_AUTOCOMPLETE_RESULT, // respond to an autocomplete interaction with suggested choices
	MODAL,                                   // respond to an interaction with a popup modal
	PREMIUM_REQUIRED                         // respond to an interaction with an upgrade button, only available for apps with monetization enabled
};

void detail::set_interaction_ack(const cInteraction& i) noexcept {
	i.m_ack = true;
}
bool detail::get_interaction_ack(const cInteraction& i) noexcept {
	return i.m_ack;
}

cTask<>
cBot::InteractionDefer(const cAppCmdInteraction& i, bool) {
	using namespace detail;
	if (get_interaction_ack(i)) throw xInteractionAcknowledgedError();
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), {{ "type", DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE }});
	set_interaction_ack(i);
}
cTask<>
cBot::InteractionDefer(const cMsgCompInteraction& i, bool bThinking) {
	using namespace detail;
	if (get_interaction_ack(i)) throw xInteractionAcknowledgedError();
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), {{ "type", bThinking ? DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE : DEFERRED_UPDATE_MESSAGE }});
	set_interaction_ack(i);
}
cTask<>
cBot::InteractionDefer(const cModalSubmitInteraction& i, bool bThinking) {
	using namespace detail;
	if (get_interaction_ack(i)) throw xInteractionAcknowledgedError();
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), {{ "type", bThinking ? DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE : DEFERRED_UPDATE_MESSAGE }});
	set_interaction_ack(i);
}
cTask<>
cBot::InteractionDefer(const cInteraction& i, bool bThinking) {
	return i.Visit([this, bThinking](auto& i) {
		return InteractionDefer(i, bThinking);
	});
}

cTask<>
cBot::InteractionSendMessage(const cInteraction& i, const cMessageParams& msg) {
	using namespace detail;
	/* If the interaction has been acknowledged before, send a followup message... */
	if (get_interaction_ack(i)) {
		co_await DiscordPost(fmt::format("/webhooks/{}/{}", i.GetApplicationId(), i.GetToken()), json::value_from(msg).get_object());
		co_return;
	}
	/* ...otherwise, do initial response... */
	json::object obj{{ "type", CHANNEL_MESSAGE_WITH_SOURCE }};
	json::value_from(msg, obj["data"]);
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), obj);
	/* ...and mark interaction as acknowledged */
	set_interaction_ack(i);
}

cTask<>
cBot::InteractionSendModal(const cAppCmdInteraction& i, const cModal& modal) {
	using namespace detail;
	if (get_interaction_ack(i)) throw xInteractionAcknowledgedError();
	json::object obj{{ "type", MODAL }};
	json::value_from(modal, obj["data"]);
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), obj);
	set_interaction_ack(i);
}
cTask<>
cBot::InteractionSendModal(const cMsgCompInteraction& i, const cModal& modal) {
	using namespace detail;
	if (get_interaction_ack(i)) throw xInteractionAcknowledgedError();
	json::object obj{{ "type", MODAL }};
	json::value_from(modal, obj["data"]);
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), obj);
	set_interaction_ack(i);
}
cTask<>
cBot::InteractionSendModal(const cInteraction& i, const cModal& modal) {
	return i.Visit(visitor{
		[](const cModalSubmitInteraction&) -> cTask<> {
			throw xInvalidFormBodyError();
			co_return;
		},
		[this, &modal](auto& i) {
			return InteractionSendModal(i, modal);
		}
	});
}

cTask<cMessage>
cBot::InteractionEditMessage(const cInteraction& i, const cMessageParams& params, crefMessage msg) {
	auto& msg_id = msg.GetId();
	json::value result = co_await DiscordPatch(fmt::format("/webhooks/{}/{}/messages/{}", i.GetApplicationId(), i.GetToken(), msg_id.ToInt() ? msg_id.ToString() : "@original"), json::value_from(params).get_object());
	co_return cMessage{ result };
}
cTask<cMessage>
cBot::InteractionGetMessage(const cInteraction& i, crefMessage msg) {
	auto& msg_id = msg.GetId();
	json::value result = co_await DiscordGet(fmt::format("/webhooks/{}/{}/messages/{}", i.GetApplicationId(), i.GetToken(), msg_id.ToInt() ? msg_id.ToString() : "@original"));
	co_return cMessage{ result };
}
cTask<>
cBot::InteractionDeleteMessage(const cInteraction& i, crefMessage msg) {
	auto &msg_id = msg.GetId();
	co_await DiscordDelete(fmt::format("/webhooks/{}/{}/messages/{}", i.GetApplicationId(), i.GetToken(), msg_id.ToInt() ? msg_id.ToString() : "@original"));
}