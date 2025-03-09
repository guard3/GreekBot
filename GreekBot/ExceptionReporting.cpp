#include "GreekBot.h"
#include "Utils.h"

namespace detail {
	[[noreturn]]
	void unhandled_exception(const char* func) {
		throw std::make_pair(func, std::current_exception());
	}

	static void report_exceptions_impl(const char* func, std::size_t level) noexcept {
		/* Print a generic error message at the current level */
		cUtils::PrintErr("{}: {:>{}}An error occurred", func, level < 2 ? "" : "╰╴", level);
	}

	static void report_exceptions_impl(const char* func, std::size_t level, const std::exception& e) noexcept {
		/* Print exception details at the current level */
		cUtils::PrintErr("{}: {:>{}}{}", func, level < 2 ? "" : "╰╴", level, e.what());
		try {
			/* If the exception is xDiscordError, print additional error details if they exist */
			if (auto p = dynamic_cast<const xDiscordError*>(&e); p && p->errors() != "null")
				cUtils::PrintErr("{}: {:>{}}Discord error details: {}", func, "╰╴", level += 2, p->errors());
			/* If there's a nested exception, rethrow and report */
			std::rethrow_if_nested(e);
		} catch (const std::pair<const char*, std::exception_ptr>& pair) {
			report_exceptions(pair.first, pair.second);
		} catch (const std::exception& nested) {
			report_exceptions_impl(func, level + 2, nested);
		} catch (...) {
			report_exceptions_impl(func, level + 2);
		}
	}

	void report_exceptions(const char* func, std::exception_ptr ex) noexcept {
		try {
			std::rethrow_exception(ex);
		} catch (const std::pair<const char*, std::exception_ptr>& e) {
			report_exceptions(e.first, e.second);
		} catch (const std::exception& e) {
			report_exceptions_impl(func, 0, e);
		} catch (...) {
			report_exceptions_impl(func, 0);
		}
	}
}