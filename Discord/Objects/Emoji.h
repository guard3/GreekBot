#ifndef _GREEKBOT_EMOJI_H_
#define _GREEKBOT_EMOJI_H_
#include "Types.h"

class cEmoji final {
private:
	std::string name;
	chSnowflake id;
	bool        animated;

public:
	explicit cEmoji(const char* name, const char* id = nullptr, bool animated = false) : name(name), id(id ? new cSnowflake(id) : nullptr), animated(animated) {}

	cEmoji(const cEmoji& e) : name(e.name), id(e.id ? new cSnowflake(*e.id) : nullptr), animated(e.animated) {}
	cEmoji(cEmoji&& e)  noexcept : name(std::move(e.name)), id(e.id), animated(e.animated) { e.id = nullptr; }
	~cEmoji() { delete id; }

	cEmoji& operator=(cEmoji e) {
		name = std::move(e.name);
		animated = e.animated;
		auto p = id;
		id = e.id;
		e.id = p;
		return *this;
	}

	[[nodiscard]] const char* GetName()    const { return name.c_str(); }
	[[nodiscard]] chSnowflake GetId()      const { return id;           }
	[[nodiscard]] bool        IsAnimated() const { return animated;     }

	[[nodiscard]] json::object ToJson() const {
		json::object obj;
		obj["name"] = name;
		if (id)
			obj["id"] = id->ToString();
		obj["animated"] = animated;
		return obj;
	}
	[[nodiscard]] std::string ToJsonString() const { return (std::stringstream() << ToJson()).str(); }
};
typedef   hHandle<cEmoji>   hEmoji;
typedef  chHandle<cEmoji>  chEmoji;
typedef  uhHandle<cEmoji>  uhEmoji;
typedef uchHandle<cEmoji> uchEmoji;
typedef  shHandle<cEmoji>  shEmoji;
typedef schHandle<cEmoji> schEmoji;
#endif /* _GREEKBOT_EMOJI_H_ */
