#ifndef DISCORD_COMPONENTBASE_H
#define DISCORD_COMPONENTBASE_H
#include "Base.h"
#include <variant>

/**
 * A base class for every component that holds the optional component identifier
 */
class cComponentBase {
	std::int32_t m_id{};

	explicit cComponentBase(std::int32_t id) noexcept : m_id(id) {}

public:
	cComponentBase() = default;

	explicit cComponentBase(const boost::json::value&);
	explicit cComponentBase(const boost::json::object&);

	/* Getters */
	std::int32_t GetId() const noexcept { return m_id; }

	/* Resetters */
	void ResetId() noexcept { m_id = 0; }

	/* Emplacers */
	std::int32_t& EmplaceId(std::uint32_t id) noexcept { return m_id = id; }

	/* Setters */
	template<typename Self>
	Self&& SetId(this Self&& self, std::int32_t id) noexcept {
		self.m_id = id;
		return std::forward<Self>(self);
	}
};

/**
 * A helper base class for variant-like components
 */
struct cVariantComponentBase {
	template<typename T, typename Self>
	auto&& As(this Self&& self) {
		return std::get<T>(std::forward<Self>(self));
	}

	template<typename T, typename Self>
	auto If(this Self&& self) {
		return cPtr(std::get_if<T>(std::addressof(self)));
	}

	template<typename Self, typename F>
	decltype(auto) Visit(this Self&& self, F&& f) {
		return std::visit(std::forward<F>(f), std::forward<Self>(self));
	}

	template<typename R, typename Self, typename F>
	R Visit(this Self&& self, F&& f) {
		return std::visit<R>(std::forward<F>(f), std::forward<Self>(self));
	}
};
#endif //DISCORD_COMPONENTBASE_H