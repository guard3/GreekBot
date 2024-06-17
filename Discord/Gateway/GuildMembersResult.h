#ifndef DISCORD_GUILDMEMBERSRESULT_H
#define DISCORD_GUILDMEMBERSRESULT_H
#include "Member.h"

class cGuildMembersChunk final {
	cSnowflake m_guild_id;
	std::size_t m_chunk_index;
	std::size_t m_chunk_count;
	std::vector<cMember> m_members;

public:
	explicit cGuildMembersChunk(const boost::json::value&);
	explicit cGuildMembersChunk(const boost::json::object&);

	const cSnowflake&        GetGuildId() const noexcept { return m_guild_id;    }
	std::size_t           GetChunkIndex() const noexcept { return m_chunk_index; }
	std::size_t           GetChunkCount() const noexcept { return m_chunk_count; }
	std::span<const cMember> GetMembers() const noexcept { return m_members;     }
	std::span<      cMember> GetMembers()       noexcept { return m_members;     }
};
#endif /* DISCORD_GUILDMEMBERSRESULT_H */