#ifndef GREEKBOT_GUILDMEMBERSRESULT_H
#define GREEKBOT_GUILDMEMBERSRESULT_H
#include "Member.h"
#include <unordered_map>
#include <coroutine>

class cGuildMembersChunk final {
private:
	cSnowflake m_guild_id;
	int m_chunk_index;
	int m_chunk_count;
	std::vector<cMember> m_members;
	std::vector<cSnowflake> m_not_found;
	uint64_t m_nonce;

public:
	cGuildMembersChunk(const json::object&);
	cGuildMembersChunk(const json::value&);

	const cSnowflake& GetGuildId() const noexcept { return m_guild_id; }
	int GetChunkIndex() const noexcept { return m_chunk_index; }
	int GetChunkCount() const noexcept { return m_chunk_count; }
	auto& GetMembers() const noexcept { return m_members; }
	auto& GetNotFound() const noexcept { return m_not_found; }
	auto GetNonce() const noexcept { return m_nonce; }

	auto MoveMembers() noexcept { return std::move(m_members); }
	auto MoveNotFound() noexcept { return std::move(m_not_found); }
};

class cGuildMembersResult final {
private:
	int m_chunk_count = 0;
	int m_chunk_total = 0;
	std::vector<cGuildMembersChunk> m_chunks;
	std::coroutine_handle<> m_handle;

public:
	void Insert(std::coroutine_handle<> h) {
		m_handle = h;
	}
	void Insert(cGuildMembersChunk c) {
		if (m_chunks.empty()) {
			m_chunks.reserve(c.GetChunkCount());
			m_chunk_total = c.GetChunkCount();
		}
		m_chunks.push_back(std::move(c));
		m_chunk_count++;
	}
	bool IsReady() const { return m_chunk_count >= m_chunk_total; }
	void Resume() const {
		if (m_handle) m_handle.resume();
	}
	cGuildMembersChunk Publish() {
		// TODO: complete
		return std::move(m_chunks.front());
	}
};


#endif //GREEKBOT_GUILDMEMBERSRESULT_H
