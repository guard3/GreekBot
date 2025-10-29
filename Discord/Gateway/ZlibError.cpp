#include "ZlibError.h"
#include <format>
#include <zlib.h>

namespace zlib {
	const std::error_category& error_category() noexcept {
		struct zlib_category : std::error_category {
			const char* name() const noexcept override { return "zlib"; }
			std::string message(int ev) const override {
				// zError should not return NULL normally, but since this is an undocumented function, it doesn't hurt to check
				auto msg = zError(ev);
				return msg && *msg ? msg : ev == Z_OK ? "no error" : std::format("unknown zlib error {}", ev);
			}
		};

		static zlib_category cat;
		return cat;
	}
}