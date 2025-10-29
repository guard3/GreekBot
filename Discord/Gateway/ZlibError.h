#ifndef DISCORD_ZLIBERROR_H
#define DISCORD_ZLIBERROR_H
#include <system_error>

namespace zlib {
	const std::error_category& error_category() noexcept;
}
#endif //DISCORD_ZLIBERROR_H
