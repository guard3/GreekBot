#ifndef GREEKBOT_APPCMDINTERACTION_H
#define GREEKBOT_APPCMDINTERACTION_H
#include "InteractionBase.h"

class cAppCmdInteraction final : public cInteraction {
private:
	cSnowflake m_id;
	eAppCmdType m_type;
	std::string m_name;
	std::optional<cSnowflake> m_guild_id;
	std::vector<cAppCmdOption> m_options;

	explicit cAppCmdInteraction(const json::object&, const json::object&);
	using cInteraction::Visit;

public:
	explicit cAppCmdInteraction(const json::value&);
	explicit cAppCmdInteraction(const json::object&);

	const cSnowflake&         GetCommandId() const noexcept { return m_id;   }
	eAppCmdType GetCommandType() const noexcept { return m_type; }
	std::string_view        GetCommandName() const noexcept { return m_name; }
	chSnowflake          GetCommandGuildId() const noexcept { return m_guild_id ? &*m_guild_id : nullptr; }

	std::span<const cAppCmdOption> GetOptions() const noexcept { return m_options; }
	std::span<      cAppCmdOption> GetOptions()       noexcept { return m_options; }

	decltype(auto) MoveCommandName() noexcept { return std::move(m_name);    }
	decltype(auto)     MoveOptions() noexcept { return std::move(m_options); }
};
typedef   hHandle<cAppCmdInteraction>   hAppCmdInteraction;
typedef  chHandle<cAppCmdInteraction>  chAppCmdInteraction;
typedef  uhHandle<cAppCmdInteraction>  uhAppCmdInteraction;
typedef uchHandle<cAppCmdInteraction> uchAppCmdInteraction;
#endif /* GREEKBOT_APPCMDINTERACTION_H */
