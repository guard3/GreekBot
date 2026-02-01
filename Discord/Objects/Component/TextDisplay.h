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

	/* Setters */
	template<typename Self>
	Self&& SetContent(this Self&& self, std::string content) noexcept {
		self.m_content = std::move(content);
		return std::forward<Self>(self);
	}
};

cTextDisplay
tag_invoke(boost::json::value_to_tag<cTextDisplay>, const boost::json::value& v);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cTextDisplay&);
#endif //DISCORD_TEXTDISPLAY_H