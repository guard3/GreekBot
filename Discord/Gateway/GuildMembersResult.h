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
	const std::vector<cMember>& GetMembers() const noexcept { return m_members; }
	std::vector<cMember>& GetMembers() noexcept { return m_members; }
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
	~cGuildMembersResult() {
		if (m_handle)
			m_handle.destroy();
	}

	void Fill(std::coroutine_handle<> h) {
		m_handle = h;
	}
	void Fill(cGuildMembersChunk c) {
		/* If no chunks have been received yet, reserve enough space in the chunk vector */
		if (m_chunks.empty())
			m_chunks.reserve(m_chunk_total = c.GetChunkCount());
		/* Save chunk */
		m_chunks.emplace_back(std::move(c));
		m_chunk_count++;
		/* If all chunks have been consumed, resume coroutine */
		if (m_chunk_count == m_chunk_total && m_handle) {
			std::coroutine_handle<> coro = m_handle;
			m_handle = nullptr;
			coro();
		}
	}

	std::vector<cMember> Publish() {
		/* If there's only one chunk, simply return its members */
		if (m_chunks.size() == 1)
			return m_chunks.front().MoveMembers();
		/* Otherwise, create a new vector with all members */
		size_t num = 0;
		for (auto& chunk : m_chunks)
			num += chunk.GetMembers().size();
		std::vector<cMember> members;
		members.reserve(num);
		for (auto& chunk : m_chunks) {
			for (cMember& m : chunk.GetMembers())
				members.emplace_back(std::move(m));
		}
		return members;
	}
};
#endif //GREEKBOT_GUILDMEMBERSRESULT_H