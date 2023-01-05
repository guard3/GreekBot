#include "GuildMembersResult.h"
#include "json.h"

cGuildMembersChunk::cGuildMembersChunk(const json::value& v) : cGuildMembersChunk(v.as_object()) {}

cGuildMembersChunk::cGuildMembersChunk(const json::object& o):
	m_guild_id(o.at("guild_id")),
	m_chunk_index(o.at("chunk_index").as_int64()),
	m_chunk_count(o.at("chunk_count").as_int64()),
	m_nonce(cUtils::ParseInt(o.at("nonce").as_string())){
	auto& m = o.at("members").as_array();
	m_members.reserve(m.size());
	for (auto& v : m)
		m_members.emplace_back(v);
	auto& n = o.at("not_found").as_array();
	m_not_found.reserve(n.size());
	for (auto& v : n)
		m_not_found.emplace_back(v);
}