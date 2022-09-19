#include "Common.h"
#include "json.h"

cSnowflake::cSnowflake(const json::value& v) : cSnowflake(v.as_string().c_str()) {}

cColor::cColor(const json::value& v) : cColor(v.as_int64()) {}