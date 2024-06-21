#include "GreekBot.h"
#include "Utils.h"
#include <algorithm>

namespace rng = std::ranges;

const cSnowflake VOICE_LOG_CHANNEL_ID = 672924470750478338;

cTask<>
cGreekBot::OnVoiceStateUpdate(cVoiceState& voice_state) {
	if (!voice_state.GetGuildId() || *voice_state.GetGuildId() != LMG_GUILD_ID)
		co_return;
	auto it = rng::find(m_lmg_voice_states, voice_state.GetUserId(), &cVoiceState::GetUserId);
	if (auto pChannelId = voice_state.GetChannelId().Get()) {
		if (it == m_lmg_voice_states.end()) {
			co_await CreateMessage(VOICE_LOG_CHANNEL_ID, cMessageParams().SetContent(fmt::format("<@{}> joined <#{}>", voice_state.GetUserId(), *pChannelId)));
			m_lmg_voice_states.push_back(std::move(voice_state));
		} else {
			if (*pChannelId != *it->GetChannelId()) {
				co_await CreateMessage(VOICE_LOG_CHANNEL_ID, cMessageParams().SetContent(fmt::format("<@{}> moved from <#{}> to <#{}>", voice_state.GetUserId(), *it->GetChannelId(), *pChannelId)));
			}
			*it = std::move(voice_state);
		}
	} else {
		if (it != m_lmg_voice_states.end()) {
			co_await CreateMessage(VOICE_LOG_CHANNEL_ID, cMessageParams().SetContent(fmt::format("<@{}> left <#{}>", voice_state.GetUserId(), *it->GetChannelId())));
			m_lmg_voice_states.erase(it);
		}
	}
}