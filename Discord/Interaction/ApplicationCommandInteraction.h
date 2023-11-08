#ifndef GREEKBOT_APPLICATIONCOMMANDINTERACTION_H
#define GREEKBOT_APPLICATIONCOMMANDINTERACTION_H
#include "InteractionBase.h"

class cApplicationCommandInteraction final : public cInteraction {
private:
	cSnowflake m_id;
	eApplicationCommandType m_type;
	std::string m_name;
	std::optional<cSnowflake> m_guild_id;
	std::vector<cApplicationCommandOption> m_options;

public:
	explicit cApplicationCommandInteraction(const json::value&);
	explicit cApplicationCommandInteraction(const json::object&);
	explicit cApplicationCommandInteraction(const json::object&, const json::object&);

	const cSnowflake&         GetCommandId() const noexcept { return m_id;   }
	eApplicationCommandType GetCommandType() const noexcept { return m_type; }
	std::string_view        GetCommandName() const noexcept { return m_name; }
	chSnowflake          GetCommandGuildId() const noexcept { return m_guild_id ? &*m_guild_id : nullptr; }

	std::span<const cApplicationCommandOption> GetOptions() const noexcept { return m_options; }
	std::span<      cApplicationCommandOption> GetOptions()       noexcept { return m_options; }

	decltype(auto) MoveCommandName() noexcept { return std::move(m_name);    }
	decltype(auto)     MoveOptions() noexcept { return std::move(m_options); }
};
#endif //GREEKBOT_APPLICATIONCOMMANDINTERACTION_H
