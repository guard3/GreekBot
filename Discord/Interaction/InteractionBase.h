#ifndef GREEKBOT_INTERACTIONBASE_H
#define GREEKBOT_INTERACTIONBASE_H
#include "Component.h"
#include "Member.h"
#include "Message.h"
#include "User.h"
#include <vector>
#include <variant>
/* ================================================================================================================== */
enum eInteractionType {
	INTERACTION_PING = 1,
	INTERACTION_APPLICATION_COMMAND,
	INTERACTION_MESSAGE_COMPONENT,
	INTERACTION_APPLICATION_COMMAND_AUTOCOMPLETE,
	INTERACTION_MODAL_SUBMIT
};
eInteractionType tag_invoke(boost::json::value_to_tag<eInteractionType>, const boost::json::value&);
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
/* ========== Forward declarations of interaction types ============================================================= */
class cAppCmdInteraction;
class cMsgCompInteraction;
class cModalSubmitInteraction;
/* ========== Concepts for valid interaction visitor types ========================================================== */
template<typename T>
concept iInteractionVisitor = requires {
	std::same_as<std::invoke_result_t<T&&, cAppCmdInteraction&>, std::invoke_result_t<T&&, cMsgCompInteraction&>>;
	std::same_as<std::invoke_result_t<T&&, cAppCmdInteraction&>, std::invoke_result_t<T&&, cModalSubmitInteraction&>>;
};
template<typename T, typename R>
concept iInteractionVisitorR = requires {
	std::convertible_to<std::invoke_result_t<T&&, cAppCmdInteraction&>, R>;
	std::convertible_to<std::invoke_result_t<T&&, cMsgCompInteraction&>, R>;
	std::convertible_to<std::invoke_result_t<T&&, cModalSubmitInteraction&>, R>;
};
/* ========== The base interaction class ============================================================================ */
class cInteraction {
private:
	cSnowflake                m_id;
	cSnowflake                m_application_id;
	std::string               m_token;
	eInteractionType          m_type;
	ePermission               m_app_permissions;
	cUser                     m_user;
	std::optional<cSnowflake> m_channel_id;

	struct guild_data {
		cSnowflake   guild_id;
		cPartialMember member;
		guild_data(const json::value&, const json::value&);
	};
	std::optional<guild_data> m_guild_data;

protected:
	cInteraction(eInteractionType, const json::object&);
	cInteraction(const cInteraction&) = default;

public:
	const cSnowflake&            GetId() const noexcept { return m_id;              }
	const cSnowflake& GetApplicationId() const noexcept { return m_application_id;  }
	std::string_view          GetToken() const noexcept { return m_token;           }
	ePermission      GetAppPermissions() const noexcept { return m_app_permissions; }
	const cUser&               GetUser() const noexcept { return m_user;            }
	chPartialMember          GetMember() const noexcept { return m_guild_data ? &m_guild_data->member   : nullptr; }
	chSnowflake             GetGuildId() const noexcept { return m_guild_data ? &m_guild_data->guild_id : nullptr; }

	cUser&           GetUser() noexcept { return m_user; }
	hPartialMember GetMember() noexcept { return m_guild_data ? &m_guild_data->member   : nullptr; }
	hSnowflake    GetGuildId() noexcept { return m_guild_data ? &m_guild_data->guild_id : nullptr; }

	template<iInteractionVisitor Visitor>
	decltype(auto) Visit(Visitor&&);
	template<iInteractionVisitor Visitor>
	decltype(auto) Visit(Visitor&&) const;
	template<typename R, iInteractionVisitorR<R> Visitor>
	R Visit(Visitor&&);
	template<typename R, iInteractionVisitorR<R> Visitor>
	R Visit(Visitor&&) const;
};
typedef   hHandle<cInteraction>   hInteraction;
typedef  chHandle<cInteraction>  chInteraction;
typedef  uhHandle<cInteraction>  uhInteraction;
typedef uchHandle<cInteraction> uchInteraction;
#endif /* GREEKBOT_INTERACTIONBASE_H */