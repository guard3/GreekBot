#pragma once
#ifndef _GREEKBOT_BOT_H_
#define _GREEKBOT_BOT_H_
#include "Interaction.h"
#include "Gateway.h"
#include "Message.h"
#include "Embed.h"
#include "Role.h"
#include <vector>
#include "User.h"
#include "Guild.h"

struct cMessageOptions {
	bool clear_content = false;
	bool clear_components = false;
	bool clear_embeds = false;
	std::string content;
	std::vector<cActionRow> components;
	std::vector<cEmbed> embeds;

	json::object ToJson(eMessageFlag flags) const {
		json::object obj {
			{   "tts", (bool)(flags & MESSAGE_FLAG_TTS) },
			{ "flags", (int)(flags & (MESSAGE_FLAG_EPHEMERAL | MESSAGE_FLAG_SUPPRESS_EMBEDS)) }
		};
		/* Set content */
		if (clear_content)
			obj["content"] = "";
		else if (!content.empty())
			obj["content"] = content;
		/* Set components */
		if (clear_components)
			obj["components"] = json::array();
		else if (!components.empty()) {
			json::array a;
			a.reserve(components.size());
			for (auto& c : components)
				a.push_back(c.ToJson());
			obj["components"] = std::move(a);
		}
		/* Set embeds */
		if (clear_embeds)
			obj["embeds"] = json::array();
		else if (!embeds.empty()) {
			json::array a;
			a.reserve(embeds.size());
			for (auto& e : embeds)
				a.push_back(e.ToJson());
			obj["embeds"] = std::move(a);
		}
		return obj;
	};
};

class cBot : public cGateway {
private:
	uhUser m_user;

	cTask<> OnReady(uhUser) override;

	std::vector<uchRole> get_guild_roles(const cSnowflake& guild_id);

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
	cTask<> RespondToInteraction(const cInteraction& interaction, eMessageFlag flags, const cMessageOptions& options = {});
	cTask<> EditInteractionResponse(const cInteraction& interaction, eMessageFlag flags, const cMessageOptions& options = {});
	cTask<> SendInteractionFollowupMessage(const cInteraction& interaction, eMessageFlag flags, const cMessageOptions& options = {});

	cTask<int> BeginGuildPrune(const cSnowflake& id, int days, std::string reason = {});
};
#endif /* _GREEKBOT_BOT_H_ */