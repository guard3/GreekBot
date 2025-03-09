#ifndef DISCORD_DISCORDERROR_H
#define DISCORD_DISCORDERROR_H
#include <memory>
#include <system_error>
#include <string_view>
/* ========== Boost forward declarations ============================================================================ */
namespace boost::json {
	class value;
}

/* ========== Discord error codes; I'm going to be filling these as I need them ===================================== */
enum class eDiscordError {
	GeneralError            = -1,
	UnknownWebhook          = 10015,
	UnknownInteraction      = 10062,
	InteractionAcknowledged = 40060,
	InvalidFormBody         = 50035
};
/* ========== Make eDiscordError implicitly convertible to std::error_code ========================================== */
template<>
struct std::is_error_code_enum<eDiscordError> : std::true_type {};

std::error_code make_error_code(eDiscordError) noexcept;

/* ========== A specialized exception type for discord error codes ================================================== */
class xDiscordError : public std::system_error {
	std::shared_ptr<char[]> m_errors;        // Use a shared pointer for the errors string to make sure xDiscordError
	std::size_t             m_errors_size{}; // is nothrow copy constructible and assignable, like all exception types

public:
	xDiscordError(eDiscordError ec);
	xDiscordError(eDiscordError ec, const char* what_arg, const boost::json::value* error = nullptr);
	xDiscordError(eDiscordError ec, const std::string& what_arg, const boost::json::value* error = nullptr);

	std::string_view errors() const noexcept;
};
#endif /* DISCORD_DISCORDERROR_H */