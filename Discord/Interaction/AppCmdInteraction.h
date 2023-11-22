#ifndef GREEKBOT_APPCMDINTERACTION_H
#define GREEKBOT_APPCMDINTERACTION_H
#include "InteractionBase.h"
#include <span>
#include <variant>
#include <vector>
/* ================================================================================================================== */
enum eAppCmdType {
	APP_CMD_CHAT_INPUT = 1,
	APP_CMD_USER,
	APP_CMD_MESSAGE
};
eAppCmdType tag_invoke(boost::json::value_to_tag<eAppCmdType>, const boost::json::value&);
/* ================================================================================================================== */
enum eAppCmdOptionType {
	APP_CMD_OPT_SUB_COMMAND = 1,
	APP_CMD_OPT_SUB_COMMAND_GROUP,
	APP_CMD_OPT_STRING,
	APP_CMD_OPT_INTEGER,
	APP_CMD_OPT_BOOLEAN,
	APP_CMD_OPT_USER,
	APP_CMD_OPT_CHANNEL,
	APP_CMD_OPT_ROLE,
	APP_CMD_OPT_MENTIONABLE,
	APP_CMD_OPT_NUMBER
};
eAppCmdOptionType tag_invoke(boost::json::value_to_tag<eAppCmdOptionType>, const boost::json::value&);
/* ================================================================================================================== */
class xAppCmdOptionTypeError : public std::invalid_argument {
public:
	xAppCmdOptionTypeError(eAppCmdOptionType);
};
/* ================================================================================================================== */
class cAppCmdOption final {
private:
	std::string       m_name;
	eAppCmdOptionType m_type;

	struct user_data {
		cUser user;
		std::optional<cPartialMember> member;
		user_data(const json::value&, const json::value*);
	};
	std::variant<int, double, bool, std::string, cSnowflake, std::vector<cAppCmdOption>, user_data> m_value;
	/* Helper functions for moving specific values */
	std::string move_value_string();
	std::pair<cUser, std::optional<cPartialMember>> move_value_user();

public:
	explicit cAppCmdOption(const json::value& v, cPtr<const json::value> r);
	explicit cAppCmdOption(eAppCmdType, std::string_view, const json::object&);
	/* Simple member getters */
	eAppCmdOptionType GetType() const noexcept { return m_type;            }
	std::string_view  GetName() const noexcept { return m_name;            }
	std::string      MoveName()       noexcept { return std::move(m_name); }
	/* Option getters for subcommands */
	std::span<const cAppCmdOption> GetOptions() const noexcept;
	std::span<cAppCmdOption>       GetOptions()       noexcept;
	std::vector<cAppCmdOption>    MoveOptions()       noexcept;
	/* Const value getters for every supported option type */
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_STRING)
	std::string_view GetValue() const;
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_INTEGER)
	int GetValue() const;
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_BOOLEAN)
	bool GetValue() const;
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_NUMBER)
	double GetValue() const;
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_CHANNEL || Type == APP_CMD_OPT_ROLE || Type == APP_CMD_OPT_MENTIONABLE)
	const cSnowflake& GetValue() const;
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_USER)
	std::pair<chUser, chPartialMember> GetValue() const;
	/* Non const value getters where it makes sense */
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_CHANNEL || Type == APP_CMD_OPT_ROLE || Type == APP_CMD_OPT_MENTIONABLE)
	cSnowflake& GetValue() {
		return const_cast<cSnowflake&>(const_cast<const cAppCmdOption*>(this)->GetValue<Type>());
	}
	template<eAppCmdOptionType Type> requires (Type == APP_CMD_OPT_USER)
	std::pair<hUser, hPartialMember> GetValue() {
		auto[u, m] = const_cast<const cAppCmdOption*>(this)->GetValue<Type>();
		return { const_cast<cUser*>(u.Get()), const_cast<cPartialMember*>(m.Get()) };
	}
	/* Value mover */
	template<eAppCmdOptionType Type> requires (Type != APP_CMD_OPT_SUB_COMMAND && Type != APP_CMD_OPT_SUB_COMMAND_GROUP)
	auto MoveValue() {
		if constexpr (Type == APP_CMD_OPT_STRING)
			return move_value_string();
		else if constexpr (Type == APP_CMD_OPT_USER)
			return move_value_user();
		else
			return GetValue<Type>();
	}
};
typedef   hHandle<cAppCmdOption>   hAppCmdOption;
typedef  chHandle<cAppCmdOption>  chAppCmdOption;
typedef  uhHandle<cAppCmdOption>  uhAppCmdOption;
typedef uchHandle<cAppCmdOption> uchAppCmdOption;
cAppCmdOption tag_invoke(boost::json::value_to_tag<cAppCmdOption>, const boost::json::value&, cPtr<const boost::json::value>);
/* ================================================================================================================== */
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
