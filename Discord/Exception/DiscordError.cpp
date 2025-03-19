#include "DiscordError.h"
#include <format>
#include <string>
#include <boost/json.hpp>
/* ========== The error category for eDiscordError ================================================================== */
class cDiscordCategory : public std::error_category {
	static const cDiscordCategory ms_instance;
public:
	static const std::error_category& GetInstance() noexcept { return ms_instance; }

	const char* name() const noexcept override {
		return "Discord";
	}
	std::string message(int ec) const override {
		return ec == 0 ? "No error" : std::format("Discord error {}", ec < 0 ? 0 : ec);
	}
};
const cDiscordCategory cDiscordCategory::ms_instance;
/* ================================================================================================================== */
std::error_code
make_error_code(eDiscordError ec) noexcept {
	return { static_cast<int>(ec), cDiscordCategory::GetInstance() };
}
/* ========== xDiscordError constructors ============================================================================ */
xDiscordError::xDiscordError(eDiscordError ec) : std::system_error(static_cast<int>(ec), cDiscordCategory::GetInstance()), m_errors_size{} {}
xDiscordError::xDiscordError(eDiscordError ec, const std::string& what_arg, const boost::json::value* error) : xDiscordError(ec, what_arg.c_str(), error) {}
xDiscordError::xDiscordError(eDiscordError ec, const char* what_arg, const boost::json::value* error) : std::system_error(static_cast<int>(ec), cDiscordCategory::GetInstance(), what_arg) {
	if (!error) return;
	/* Create a serializer for the supplied error object */
	boost::json::serializer sr;
	sr.reset(error);
	/* Initialize a string with 0 size and 256 capacity */
	std::size_t size = 0, capacity = 256;
	auto str = std::make_shared_for_overwrite<char[]>(capacity);
	/* Keep reading characters, reallocating when necessary */
	for (;;) {
		size += sr.read(str.get() + size, capacity - size).size();
		if (sr.done())
			break;
		capacity *= 2;
		auto buff = std::make_shared_for_overwrite<char[]>(capacity);
		std::copy_n(str.get(), size, buff.get());
		str = std::move(buff);
	}
	/* Save the final results */
	m_errors = std::move(str);
	m_errors_size = size;
}
/* ================================================================================================================== */
std::string_view
xDiscordError::errors() const noexcept {
	return m_errors ? std::string_view(m_errors.get(), m_errors_size) : "null";
}