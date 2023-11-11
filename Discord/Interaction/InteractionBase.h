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
class xInvalidAttributeError : public std::runtime_error {
public:
	xInvalidAttributeError(const char* m)        : std::runtime_error(m) {}
	xInvalidAttributeError(const std::string& m) : std::runtime_error(m) {}
};
/* ================================================================================================================== */
class cAppCmdOption final {
private:
	template<eAppCmdOptionType> struct a;

	std::string       m_name;
	eAppCmdOptionType m_type;

	std::variant<std::monostate,
		std::vector<cAppCmdOption>,
		std::tuple<cUser, uhMember>,
		std::string,
		int,
		bool,
		double,
		cSnowflake> m_value;
	
public:
	explicit cAppCmdOption(const json::value& v, cPtr<const json::value> r);
	explicit cAppCmdOption(eAppCmdType, std::string_view, const json::object&);

	/* Return types */
	template<eAppCmdOptionType t>
	using tMoveType = typename a<t>::value_type;
	template<eAppCmdOptionType t>
	using tValueType = std::conditional_t<t == APP_CMD_OPT_INTEGER || t == APP_CMD_OPT_BOOLEAN || t == APP_CMD_OPT_NUMBER, tMoveType<t>, tMoveType<t>&>;
	template<eAppCmdOptionType t>
	using tConstValueType = std::conditional_t<std::is_reference_v<tValueType<t>>, const tMoveType<t>&, tMoveType<t>>;
	/* Access type */
	eAppCmdOptionType GetType() const noexcept { return m_type; }
	/* Access name */
	std::string& GetName() noexcept { return m_name; }
	const std::string& GetName() const noexcept { return m_name; }
	std::string MoveName() noexcept { return std::move(m_name); }
	/* Access options - subcommands only */
	std::vector<cAppCmdOption>& GetOptions();
	const std::vector<cAppCmdOption>& GetOptions() const {
		return const_cast<cAppCmdOption*>(this)->GetOptions();
	}
	std::vector<cAppCmdOption> MoveOptions() {
		return std::move(GetOptions());
	}
	/* Access value */
	template<eAppCmdOptionType t> requires (t != APP_CMD_OPT_SUB_COMMAND && t != APP_CMD_OPT_SUB_COMMAND_GROUP)
	tValueType<t> GetValue() {
		try {
			if constexpr (t == APP_CMD_OPT_USER)
				return std::get<0>(std::get<2>(m_value));
			else
				return std::get<tMoveType<t>>(m_value);
		}
		catch (const std::bad_variant_access&) {
			throw xInvalidAttributeError(fmt::format("Application command option is not of type {}", a<t>::name));
		}
	}
	template<eAppCmdOptionType t> requires (t != APP_CMD_OPT_SUB_COMMAND && t != APP_CMD_OPT_SUB_COMMAND_GROUP)
	tConstValueType<t> GetValue() const {
		return const_cast<cAppCmdOption*>(this)->template GetValue<t>();
	}
	template<eAppCmdOptionType t>
	tMoveType<t> MoveValue() { return std::move(GetValue<t>()); }
	/* Access member */
	hMember GetMember();
	chMember GetMember() const { return const_cast<cAppCmdOption*>(this)->GetMember(); }
	uhMember MoveMember();
};
template<> struct cAppCmdOption::a<APP_CMD_OPT_STRING>      { using value_type = std::string; static inline auto name = "APP_CMD_OPT_STRING";      };
template<> struct cAppCmdOption::a<APP_CMD_OPT_INTEGER>     { using value_type = int;         static inline auto name = "APP_CMD_OPT_INTEGER";     };
template<> struct cAppCmdOption::a<APP_CMD_OPT_BOOLEAN>     { using value_type = bool;        static inline auto name = "APP_CMD_OPT_BOOLEAN";     };
template<> struct cAppCmdOption::a<APP_CMD_OPT_USER>        { using value_type = cUser;       static inline auto name = "APP_CMD_OPT_USER";        };
template<> struct cAppCmdOption::a<APP_CMD_OPT_CHANNEL>     { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_CHANNEL";     };
template<> struct cAppCmdOption::a<APP_CMD_OPT_ROLE>        { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_ROLE";        };
template<> struct cAppCmdOption::a<APP_CMD_OPT_MENTIONABLE> { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_MENTIONABLE"; };
template<> struct cAppCmdOption::a<APP_CMD_OPT_NUMBER>      { using value_type = double;      static inline auto name = "APP_CMD_OPT_NUMBER";      };

typedef   hHandle<cAppCmdOption>   hApplicationCommandInteractionDataOption;
typedef  chHandle<cAppCmdOption>  chApplicationCommandInteractionDataOption;
typedef  uhHandle<cAppCmdOption>  uhApplicationCommandInteractionDataOption;
typedef uchHandle<cAppCmdOption> uchApplicationCommandInteractionDataOption;
typedef  uhHandle<cAppCmdOption>  shApplicationCommandInteractionDataOption;
typedef uchHandle<cAppCmdOption> schApplicationCommandInteractionDataOption;
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