#ifndef DISCORD_ROLE_H
#define DISCORD_ROLE_H
#include "Permission.h"
#include "RoleFwd.h"
#include <optional>

class cRoleTags final {
	cSnowflake bot_id;
	cSnowflake integration_id;
	// various other crap...

public:
	cRoleTags(const boost::json::value&);
	cRoleTags(const boost::json::object&);

	chSnowflake         GetBotId() const noexcept { return         bot_id.ToInt() ?         &bot_id : nullptr; }
	chSnowflake GetIntegrationId() const noexcept { return integration_id.ToInt() ? &integration_id : nullptr; }
};

class cRole final {
	cSnowflake  m_id;
	std::size_t m_position;
	ePermission m_permissions;
	cColor      m_color;
	bool        m_hoist;
	bool        m_managed;
	bool        m_mentionable;
	std::string m_name;
	std::string m_icon;
	std::string m_unicode_emoji;
	std::optional<cRoleTags> m_tags;

public:
	cRole(const boost::json::value&);
	cRole(const boost::json::object&);

	const cSnowflake&          GetId() const noexcept { return m_id;            }
	std::size_t          GetPosition() const noexcept { return m_position;      }
	ePermission       GetPermissions() const noexcept { return m_permissions;   }
	cColor                  GetColor() const noexcept { return m_color;         }
	std::string_view         GetName() const noexcept { return m_name;          }
	std::string_view         GetIcon() const noexcept { return m_icon;          }
	std::string_view GetUnicodeEmoji() const noexcept { return m_unicode_emoji; }
	chRoleTags               GetTags() const noexcept { return m_tags ? m_tags.operator->() : nullptr; }

	bool     IsHoisted() const noexcept { return m_hoist;       }
	bool     IsManaged() const noexcept { return m_managed;     }
	bool IsMentionable() const noexcept { return m_mentionable; }

	/**
	 * Allow implicit conversion to crefRole
	 */
	operator crefRole() const noexcept { return m_id; }
};

cRole tag_invoke(boost::json::value_to_tag<cRole>, const boost::json::value&);
#endif /* DISCORD_ROLE_H */