#ifndef GREEKBOT_EMOJI_H
#define GREEKBOT_EMOJI_H
#include "Common.h"

//TODO: separate guild emoji and standard emoji types
class cEmoji final {
private:
	std::string m_name;
	cSnowflake  m_id;
	bool        m_animated;

public:
	cEmoji(std::string name, const cSnowflake& id, bool animated = false):
		m_name(std::move(name)),
		m_id(id),
		m_animated(animated) {}

	const std::string& GetName() const noexcept { return m_name;     }
	const cSnowflake&    GetId() const noexcept { return m_id;       }
	bool            IsAnimated() const noexcept { return m_animated; }

	std::string ToString() const { return cUtils::Format("<%s:%s:%s>", m_animated ? "a" : "", m_name, m_id.ToString());}
	json::object ToJson() const;
};
typedef   hHandle<cEmoji>   hEmoji;
typedef  chHandle<cEmoji>  chEmoji;
typedef  uhHandle<cEmoji>  uhEmoji;
typedef uchHandle<cEmoji> uchEmoji;
typedef  shHandle<cEmoji>  shEmoji;
typedef schHandle<cEmoji> schEmoji;
#endif //GREEKBOT_EMOJI_H
