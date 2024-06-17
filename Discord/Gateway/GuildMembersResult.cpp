#include "GuildMembersResult.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
cGuildMembersChunk::cGuildMembersChunk(const json::value& v): cGuildMembersChunk(v.as_object()) {}
cGuildMembersChunk::cGuildMembersChunk(const json::object& o):
	m_guild_id(o.at("guild_id").as_string()),
	m_chunk_index(o.at("chunk_index").to_number<std::size_t>()),
	m_chunk_count(o.at("chunk_count").to_number<std::size_t>()),
	m_members(json::value_to<std::vector<cMember>>(o.at("members"))) {}