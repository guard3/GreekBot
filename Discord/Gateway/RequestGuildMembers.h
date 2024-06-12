#ifndef DISCORD_REQUESTGUILDMEMBERS_H
#define DISCORD_REQUESTGUILDMEMBERS_H
#include "Common.h"
#include <span>
#include <vector>

class cRequestGuildMembers final {
	std::string m_query;
	std::vector<cSnowflake> m_user_ids;
public:
	/* Getters */
	std::string_view GetQuery() const noexcept { return m_query; }
	std::span<const cSnowflake> GetUserIds() const noexcept { return m_user_ids; }
	/* Movers */
	std::string MoveQuery() noexcept { return std::move(m_query); }
	std::vector<cSnowflake> MoveUserIds() noexcept { return std::move(m_user_ids); }
	/* Resetters */
	void ResetQuery() noexcept { m_query.clear(); }
	void ResetUserIds() noexcept { m_user_ids.clear(); }
	/* Emplacers */
	std::string& EmplaceQuery() noexcept {
		m_query.clear();
		return m_query;
	}
	std::vector<cSnowflake>& EmplaceUserIds() noexcept {
		m_user_ids.clear();
		return m_user_ids;
	}
	template<typename T = decltype(m_query), typename Arg = T, typename... Args> requires std::constructible_from<T, Arg&&, Args&&...>
	T& EmplaceQuery(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<T&, Arg&&> && sizeof...(args) == 0)
			return m_query = std::forward<Arg>(arg);
		else
			return m_query = T(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename T = decltype(m_user_ids), typename Arg = T, typename... Args> requires std::constructible_from<T, Arg&&, Args&&...>
	T& EmplaceUserIds(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<T&, Arg&&> && sizeof...(args) == 0)
			return m_user_ids = std::forward<Arg>(arg);
		else
			return m_user_ids = T(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	/* Setters */
	template<typename T = decltype(m_query), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cRequestGuildMembers& SetQuery(Arg&& arg) & { EmplaceQuery(std::forward<Arg>(arg)); return *this; }
	template<typename T = decltype(m_user_ids), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cRequestGuildMembers& SetUserIds(Arg&& arg) & { EmplaceUserIds(std::forward<Arg>(arg)); return *this; }
	template<typename T = decltype(m_query), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cRequestGuildMembers&& SetQuery(Arg&& arg) && { return std::move(SetQuery(std::forward<Arg>(arg))); }
	template<typename T = decltype(m_user_ids), typename Arg = T> requires std::constructible_from<T, Arg&&>
	cRequestGuildMembers&& SetUserIds(Arg&& arg) && { return std::move(SetUserIds(std::forward<Arg>(arg))); }
};
using   hRequestGuildMembers =   hHandle<cRequestGuildMembers>;
using  chRequestGuildMembers =  chHandle<cRequestGuildMembers>;
using  uhRequestGuildMembers =  uhHandle<cRequestGuildMembers>;
using uchRequestGuildMembers = uchHandle<cRequestGuildMembers>;
#endif /* DISCORD_REQUESTGUILDMEMBERS_H */