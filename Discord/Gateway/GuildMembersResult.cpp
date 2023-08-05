#include "GuildMembersResult.h"
#include "Utils.h"
#include "json.h"

cGuildMembersChunk::cGuildMembersChunk(const json::value& v) : cGuildMembersChunk(v.as_object()) {}

cGuildMembersChunk::cGuildMembersChunk(const json::object& o):
	m_guild_id(json::value_to<cSnowflake>(o.at("guild_id"))),
	m_chunk_index(o.at("chunk_index").to_number<int>()),
	m_chunk_count(o.at("chunk_count").to_number<int>()),
	m_nonce(cUtils::ParseInt(json::value_to<std::string_view>(o.at("nonce")))),
	m_not_found(json::value_to<std::vector<cSnowflake>>(o.at("not_found"))),
	m_members(json::value_to<std::vector<cMember>>(o.at("members"))) {}