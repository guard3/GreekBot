#ifndef GREEKBOT_INTERACTIONBASE_H
#define GREEKBOT_INTERACTIONBASE_H
#include "Member.h"
#include "User.h"
#include <optional>
/* ================================================================================================================== */
enum eInteractionType {
	INTERACTION_PING = 1,
	INTERACTION_APPLICATION_COMMAND,
	INTERACTION_MESSAGE_COMPONENT,
	INTERACTION_APPLICATION_COMMAND_AUTOCOMPLETE,
	INTERACTION_MODAL_SUBMIT
};
eInteractionType tag_invoke(boost::json::value_to_tag<eInteractionType>, const boost::json::value&);
/* ========== Forward declarations of interaction types ============================================================= */
class cAppCmdInteraction;
class cMsgCompInteraction;
class cModalSubmitInteraction;
/* ========== Concepts for valid interaction visitor types ========================================================== */
template<typename T>
concept iInteractionVisitor = requires {
	requires std::same_as<std::invoke_result_t<T&&, cAppCmdInteraction&>, std::invoke_result_t<T&&, cMsgCompInteraction&>>;
	requires std::same_as<std::invoke_result_t<T&&, cAppCmdInteraction&>, std::invoke_result_t<T&&, cModalSubmitInteraction&>>;
};
template<typename T, typename R>
concept iInteractionVisitorR = requires {
	requires std::convertible_to<std::invoke_result_t<T&&, cAppCmdInteraction&>, R>;
	requires std::convertible_to<std::invoke_result_t<T&&, cMsgCompInteraction&>, R>;
	requires std::convertible_to<std::invoke_result_t<T&&, cModalSubmitInteraction&>, R>;
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