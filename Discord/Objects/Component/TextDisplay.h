#ifndef DISCORD_TEXTDISPLAY_H
#define DISCORD_TEXTDISPLAY_H
#include "ComponentBase.h"

class cTextDisplay : public cComponentBase {
	std::string m_content;

public:
	cTextDisplay() = default;
	template<iExplicitlyConvertibleTo<std::string> Str = std::string>
	explicit cTextDisplay(Str&& content) : m_content(std::forward<Str>(content)) {}

	explicit cTextDisplay(const boost::json::value&);
	explicit cTextDisplay(const boost::json::object&);

	/* Getters */
	std::string_view GetContent() const noexcept { return m_content; }

	/* Movers */
	std::string MoveContent() noexcept { return std::move(m_content); }

	/* Emplacers */
	std::string& EmplaceContent() noexcept {
		m_content.clear();
		return m_content;
	}
	template<typename Arg, typename... Args>
	std::string& EmplaceContent(Arg&& arg, Args&&... args) {
		if constexpr (sizeof...(args) == 0 && std::is_assignable_v<std::string&, Arg&&>) {
			return m_content = std::forward<Arg>(arg);
		} else {
			return m_content = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
		}
	}

	/* Setters */
	template<typename Self, typename Arg>
	Self&& SetContent(this Self&& self, Arg&& content) noexcept {
		self.EmplaceContent(std::forward<Arg>(content));
		return std::forward<Self>(self);
	}
};

cTextDisplay
tag_invoke(boost::json::value_to_tag<cTextDisplay>, const boost::json::value& v);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cTextDisplay&);
#endif //DISCORD_TEXTDISPLAY_H