#ifndef GREEKBOT_INTERACTION_H
#define GREEKBOT_INTERACTION_H
#include "User.h"
#include "Member.h"
#include <vector>
#include <variant>
#include "Component.h"
#include "Message.h"
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
enum eApplicationCommandType {
	APP_CMD_CHAT_INPUT = 1,
	APP_CMD_USER,
	APP_CMD_MESSAGE
};
eApplicationCommandType tag_invoke(boost::json::value_to_tag<eApplicationCommandType>, const boost::json::value&);
/* ================================================================================================================== */
enum eApplicationCommandOptionType {
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
eApplicationCommandOptionType tag_invoke(boost::json::value_to_tag<eApplicationCommandOptionType>, const boost::json::value&);
/* ================================================================================================================== */
class xInvalidAttributeError : public std::runtime_error {
public:
	xInvalidAttributeError(const char* m)        : std::runtime_error(m) {}
	xInvalidAttributeError(const std::string& m) : std::runtime_error(m) {}
};
/* ================================================================================================================== */
class cApplicationCommandOption final {
private:
	template<eApplicationCommandOptionType> struct a;

	std::string                   m_name;
	eApplicationCommandOptionType m_type;

	std::variant<std::monostate,
		std::vector<cApplicationCommandOption>,
		std::tuple<cUser, uhMember>,
		std::string,
		int,
		bool,
		double,
		cSnowflake> m_value;
	
public:
	cApplicationCommandOption(const json::value& v, cPtr<const json::value> r);

	/* Return types */
	template<eApplicationCommandOptionType t>
	using tMoveType = typename a<t>::value_type;
	template<eApplicationCommandOptionType t>
	using tValueType = std::conditional_t<t == APP_CMD_OPT_INTEGER || t == APP_CMD_OPT_BOOLEAN || t == APP_CMD_OPT_NUMBER, tMoveType<t>, tMoveType<t>&>;
	template<eApplicationCommandOptionType t>
	using tConstValueType = std::conditional_t<std::is_reference_v<tValueType<t>>, const tMoveType<t>&, tMoveType<t>>;
	/* Access type */
	eApplicationCommandOptionType GetType() const noexcept { return m_type; }
	/* Access name */
	std::string& GetName() noexcept { return m_name; }
	const std::string& GetName() const noexcept { return m_name; }
	std::string MoveName() noexcept { return std::move(m_name); }
	/* Access options - subcommands only */
	std::vector<cApplicationCommandOption>& GetOptions();
	const std::vector<cApplicationCommandOption>& GetOptions() const {
		return const_cast<cApplicationCommandOption*>(this)->GetOptions();
	}
	std::vector<cApplicationCommandOption> MoveOptions() {
		return std::move(GetOptions());
	}
	/* Access value */
	template<eApplicationCommandOptionType t> requires (t != APP_CMD_OPT_SUB_COMMAND && t != APP_CMD_OPT_SUB_COMMAND_GROUP)
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
	template<eApplicationCommandOptionType t> requires (t != APP_CMD_OPT_SUB_COMMAND && t != APP_CMD_OPT_SUB_COMMAND_GROUP)
	tConstValueType<t> GetValue() const {
		return const_cast<cApplicationCommandOption*>(this)->template GetValue<t>();
	}
	template<eApplicationCommandOptionType t>
	tMoveType<t> MoveValue() { return std::move(GetValue<t>()); }
	/* Access member */
	hMember GetMember();
	chMember GetMember() const { return const_cast<cApplicationCommandOption*>(this)->GetMember(); }
	uhMember MoveMember();
};
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_STRING>      { using value_type = std::string; static inline auto name = "APP_CMD_OPT_STRING";      };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_INTEGER>     { using value_type = int;         static inline auto name = "APP_CMD_OPT_INTEGER";     };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_BOOLEAN>     { using value_type = bool;        static inline auto name = "APP_CMD_OPT_BOOLEAN";     };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_USER>        { using value_type = cUser;       static inline auto name = "APP_CMD_OPT_USER";        };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_CHANNEL>     { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_CHANNEL";     };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_ROLE>        { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_ROLE";        };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_MENTIONABLE> { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_MENTIONABLE"; };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_NUMBER>      { using value_type = double;      static inline auto name = "APP_CMD_OPT_NUMBER";      };

typedef   hHandle<cApplicationCommandOption>   hApplicationCommandInteractionDataOption;
typedef  chHandle<cApplicationCommandOption>  chApplicationCommandInteractionDataOption;
typedef  uhHandle<cApplicationCommandOption>  uhApplicationCommandInteractionDataOption;
typedef uchHandle<cApplicationCommandOption> uchApplicationCommandInteractionDataOption;
typedef  uhHandle<cApplicationCommandOption>  shApplicationCommandInteractionDataOption;
typedef uchHandle<cApplicationCommandOption> schApplicationCommandInteractionDataOption;

class cModalSubmitData final {
private:
	std::string m_custom_id;
	std::string m_value;

public:
	explicit cModalSubmitData(const json::value&);

	std::string_view GetCustomId() const noexcept { return m_custom_id; }
	std::string_view GetValue()    const noexcept { return m_value;     }
};

/* ================================================================================================================== */
template<eInteractionType>
class cInteractionData;
/* ================================================================================================================== */
template<>
class cInteractionData<INTERACTION_APPLICATION_COMMAND> final {
private:
	cSnowflake              id;        // The id of the invoked command
	std::string             name;      // The name of the invoked command
	eApplicationCommandType type;      // The type of the invoked command
	chSnowflake             target_id; // The id the of user or message targeted by a user or message command

public:
	std::vector<cApplicationCommandOption> Options;

	cInteractionData(const json::object&);
	cInteractionData(const json::value&);

	const cSnowflake&       GetCommandId() const { return id;        }
	const std::string&      GetName()      const { return name;      }
	eApplicationCommandType GetType()      const { return type;      }
	chSnowflake             GetTargetId()  const { return target_id; }
};
/* ================================================================================================================== */
template<>
class cInteractionData<INTERACTION_MESSAGE_COMPONENT> final {
private:
	std::string    custom_id;
	eComponentType component_type;

public:
	std::vector<std::string> Values;

	cInteractionData(const json::object&);
	cInteractionData(const json::value&);

	const std::string&  GetCustomId() const noexcept { return custom_id;      }
	eComponentType GetComponentType() const noexcept { return component_type; }
};
template<eInteractionType e> using   hInteractionData =   hHandle<cInteractionData<e>>;
template<eInteractionType e> using  chInteractionData =  chHandle<cInteractionData<e>>;
template<eInteractionType e> using  uhInteractionData =  uhHandle<cInteractionData<e>>;
template<eInteractionType e> using uchInteractionData = uchHandle<cInteractionData<e>>;
template<eInteractionType e> using  shInteractionData =  shHandle<cInteractionData<e>>;
template<eInteractionType e> using schInteractionData = schHandle<cInteractionData<e>>;

template<>
class cInteractionData<INTERACTION_MODAL_SUBMIT> {
private:
	std::string m_custom_id;
	std::vector<cModalSubmitData> m_submit;

public:
	cInteractionData(const json::value&);

	std::string_view            GetCustomId()   const noexcept { return m_custom_id; }
	std::span<const cModalSubmitData> GetSubmittedData() const noexcept { return m_submit; }
};
/* ================================================================================================= */
class cInteraction final {
private:
	cSnowflake       m_id;
	cSnowflake       m_application_id;
	std::string      m_token;
	int              m_version;
	eInteractionType m_type;
	std::optional<cSnowflake> m_guild_id;
	std::optional<cSnowflake> m_channel_id;
	std::optional<cMessage>   m_message;
	std::variant<std::monostate,
		cInteractionData<INTERACTION_APPLICATION_COMMAND>,
		cInteractionData<INTERACTION_MESSAGE_COMPONENT>,
		cInteractionData<INTERACTION_MODAL_SUBMIT>> m_data;
	std::variant<std::monostate,
		cUser,
		cMember> m_um;

public:
	explicit cInteraction(const json::object&);
	explicit cInteraction(const json::value&);
	
	const cSnowflake&  GetId()            const noexcept { return m_id;             }
	const cSnowflake&  GetApplicationId() const noexcept { return m_application_id; }
	const std::string& GetToken()         const noexcept { return m_token;          }
	eInteractionType   GetType()          const noexcept { return m_type;           }
	int                GetVersion()       const noexcept { return m_version;        }
	chUser             GetUser()          const noexcept { return std::get_if<1>(&m_um); }
	chMember           GetMember()        const noexcept { return std::get_if<2>(&m_um); }
	chSnowflake        GetGuildId()       const noexcept { return m_guild_id   ? &m_guild_id.value()   : nullptr; }
	chSnowflake        GetChannelId()     const noexcept { return m_channel_id ? &m_channel_id.value() : nullptr; }
	chMessage          GetMessage()       const noexcept { return m_message    ? &m_message.value()    : nullptr; }

	template<eInteractionType t> requires (t == INTERACTION_APPLICATION_COMMAND || t == INTERACTION_MESSAGE_COMPONENT || t == INTERACTION_MODAL_SUBMIT)
	const cInteractionData<t>& GetData() const { return std::get<cInteractionData<t>>(m_data); }
};
typedef   hHandle<cInteraction>   hInteraction; // handle
typedef  chHandle<cInteraction>  chInteraction; // const handle
typedef  uhHandle<cInteraction>  uhInteraction; // unique handle
typedef uchHandle<cInteraction> uchInteraction; // unique const handle
typedef  shHandle<cInteraction>  shInteraction; // shared handle
typedef schHandle<cInteraction> schInteraction; // shared const handle
#endif //GREEKBOT_INTERACTION_H