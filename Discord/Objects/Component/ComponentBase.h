#ifndef DISCORD_COMPONENTBASE_H
#define DISCORD_COMPONENTBASE_H
#include "Base.h"

class cComponentBase {
	std::int32_t m_id{}; // Optional identifier for component

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
#endif //DISCORD_COMPONENTBASE_H