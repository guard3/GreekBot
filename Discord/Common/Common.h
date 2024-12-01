#ifndef DISCORD_COMMON_H
#define DISCORD_COMMON_H
#include "Ptr.h"
#include <cstdint>
#include <memory>

#define DISCORD_STR_(x) #x
#define DISCORD_STR(x) DISCORD_STR_(x)

#define DISCORD_API_VERSION     10
#define DISCORD_API_VERSION_STR DISCORD_STR(DISCORD_API_VERSION)
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" DISCORD_API_VERSION_STR

/* ========== Boost/JSON forward declarations ======================================================================= */
namespace boost::json {
	class value;
	class object;

	template<typename>
	struct value_to_tag;
	struct value_from_tag;

	template<typename>
	struct is_variant_like;
}
/* ========== Handle types ========================================================================================== */
template<typename T> // handle
using   hHandle = cPtr<T>;
template<typename T> // const handle
using  chHandle = cPtr<const T>;
template<typename T> // unique handle
using  uhHandle = std::unique_ptr<T>;
template<typename T> // unique const handle
using uchHandle = std::unique_ptr<const T>;
#endif /* DISCORD_COMMON_H */