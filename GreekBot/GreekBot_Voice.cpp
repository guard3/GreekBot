#include "GreekBot.h"
#include "Utils.h"
#include "CDN.h"
#include <algorithm>

namespace rng = std::ranges;

const cSnowflake VOICE_LOG_CHANNEL_ID = 672924470750478338;

cTask<>
cGreekBot::OnVoiceStateUpdate(cVoiceState& voice_state) {
	using namespace std::chrono;
	/* Make sure we're in Learning Greek */
	if (!voice_state.GetGuildId() || *voice_state.GetGuildId() != LMG_GUILD_ID)
		co_return;
	/* Find the voice state in the list, if it exists */
	auto it = rng::find(m_lmg_voice_states, voice_state.GetUserId(), &cVoiceState::GetUserId);
	/* Prepare the log message */
	auto& user = *voice_state.GetMember()->GetUser();
	cPartialMessage msg;
	auto& embed = msg.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(user.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(user));
	embed.SetTimestamp(floor<milliseconds>(system_clock::now()));
	auto& fields = embed.EmplaceFields();
	fields.reserve(3);
	fields.emplace_back("User ID", fmt::format("`{}`", user.GetId()), true);
	/* Check if there's anything to log */
	if (auto pChannelId = voice_state.GetChannelId().Get(); !pChannelId) {
		/* If there's no channel id, then the user disconnected from a channel */
		embed.SetColor(0xE91D63);
		if (it == m_lmg_voice_states.end()) {
			/* This shouldn't ever be true, but we check just for good measure */
			embed.SetDescription(fmt::format("<:vc_leave:1253820278816112720> Left a voice channel"));
		} else {
			embed.SetDescription(fmt::format("<:vc_leave:1253820278816112720> Left <#{}>", *it->GetChannelId()));
			fields.emplace_back("Channel ID", fmt::format("`{}`", *it->GetChannelId()), true);
			/* Don't forget to remove the voice state from the list */
			m_lmg_voice_states.erase(it);
		}
	} else if (it == m_lmg_voice_states.end()) {
		/* If there's no voice state for the user, that means they just connected to the channel */
		embed.SetColor(0x3598DC);
		embed.SetDescription(fmt::format("<:vc_join:1253810834950586472> Joined <#{}>", *pChannelId));
		fields.emplace_back("Channel ID", fmt::format("`{}`", *pChannelId), true);
		/* Add the voice state to the list */
		m_lmg_voice_states.push_back(std::move(voice_state));
	} else if (auto& old_channel_id = *it->GetChannelId(); *pChannelId != old_channel_id) {
		/* If the channel id changed, that means the user moved to another channel */
		embed.SetColor(0x9B59B6);
		embed.SetDescription(fmt::format("<:vc_move:1253816785405349948> Moved from <#{}> to <#{}>", old_channel_id, *pChannelId));
		fields.emplace_back("Old Channel ID", fmt::format("`{}`", old_channel_id), true);
		fields.emplace_back("New Channel ID", fmt::format("`{}`", *pChannelId), true);
		/* Update the cached voice state */
		*it = std::move(voice_state);
	} else {
		/* Otherwise, just update the cached voice state and exit */
		*it = std::move(voice_state);
		co_return;
	}
	/* Send log message */
	co_await CreateMessage(VOICE_LOG_CHANNEL_ID, msg);
}