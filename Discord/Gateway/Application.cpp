#include "Application.h"
#include "json.h"

cApplication::cApplication(const json::value& v) : cApplication(v.as_object()) {}
cApplication::cApplication(const json::object& o) : m_id(o.at("id")), m_flags((eApplicationFlag)o.at("flags").as_int64()) {}