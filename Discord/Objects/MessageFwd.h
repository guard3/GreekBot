#ifndef DISCORD_MESSAGEFWD_H
#define DISCORD_MESSAGEFWD_H
#include "BaseFwd.h"
#include <variant>

/**
 * Forward declarations
 */
DISCORD_FWDDECL_CLASS(MessageUpdate);
DISCORD_FWDDECL_CLASS(PartialMessage);
DISCORD_FWDDECL_CLASS(PartialMessageV2);
DISCORD_FWDDECL_CLASS(Message);

/**
 * A simple const reference wrapper around either a message or a snowflake
 */
struct crefMessage : crefBase {
	using crefBase::crefBase;
};

/**
 * A non-owning view to a message
 */
class cMessageView final {
	std::variant<const cPartialMessage*, const cPartialMessageV2*> m_msg;

public:
	cMessageView(const cPartialMessage& msg) noexcept : m_msg(&msg) {}
	cMessageView(const cPartialMessageV2& msg) noexcept : m_msg(&msg) {}

	template<typename F>
	decltype(auto) Visit(this cMessageView self, F&& f);

	template<typename R, typename F>
	decltype(auto) Visit(this cMessageView self, F&& f);
};
#endif /* DISCORD_MESSAGEFWD_H */