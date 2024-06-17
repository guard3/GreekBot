#include "GatewayImpl.h"
#include "Utils.h"

void
cGateway::implementation::rgm_reset() {
	/* Generate an exception for canceled guild member requests */
	try {
		throw xGatewaySessionResetError();
	} catch (...) {
		m_rgm_exception = std::current_exception();
	}
	/* Cancel all pending requests that didn't manage to get sent */
	while (!m_pending.empty()) {
		m_pending.front().resume();
		m_pending.pop_front();
	}
	/* Cancel all ongoing requests;
	 * Range for is safe because potential erasing of entries occurs by means of asio::defer */
	for (auto& elem : m_rgm_entries) {
		auto& entry = elem.second;
		entry.except = m_rgm_exception;
		if (auto coro = entry.coro)
			coro.resume();
	}
	m_rgm_exception = {};
}

void
cGateway::implementation::rgm_timeout() {
	if (m_rgm_entries.empty())
		return;
	auto now = chrono::system_clock::now();
	/* Range for is safe because potential erasing of entries occurs by means of asio::defer */
	std::exception_ptr ex;
	for (auto& elem : m_rgm_entries) {
		auto& entry = elem.second;
		if (entry.coro && now - entry.started_at > chrono::minutes(2)) {
			if (!ex) try {
				throw xGatewayTimeoutError();
			} catch (...) {
				ex = std::current_exception();
			}
			entry.except = ex;
			entry.coro.resume();
		}
	}
}

cAsyncGenerator<cMember>
cGateway::implementation::RequestGuildMembers(const cSnowflake& guild_id) {
	return request_guild_members(guild_id, {}, {});
}
cAsyncGenerator<cMember>
cGateway::implementation::RequestGuildMembers(const cSnowflake& guild_id, const cRequestGuildMembers& rgm) {
	return request_guild_members(guild_id, rgm.GetQuery(), rgm.GetUserIds());
}
cAsyncGenerator<cMember>
cGateway::implementation::request_guild_members(const cSnowflake& guild_id, std::string_view query, std::span<const cSnowflake> user_ids) {
	co_await ResumeOnWebSocketStrand();
	/* Wait for the websocket stream to become available */
	co_await [this] {
		struct _ {
			implementation* self;
			bool await_ready() const noexcept { return !(self->m_async_status & ASYNC_CLOSE); }
			void await_suspend(std::coroutine_handle<> h) const { self->m_pending.push_back(h); }
			void await_resume() const {
				if (self->m_rgm_exception)
					std::rethrow_exception(self->m_rgm_exception);
			}
		}; return _{ this };
	}();
	/* Back up the current nonce and increment the original for future use */
	auto nonce = m_rgm_nonce++;
	/* Send payload */
	send([&guild_id, query, user_ids, nonce] {
		json::object obj;
		obj.reserve(2);
		obj.emplace("op", 8);
		auto& data = obj["d"].emplace_object();
		data.reserve(3);
		json::value_from(guild_id, data["guild_id"]);
		data.emplace("limit", user_ids.size());
		if (user_ids.empty())
			data.emplace("query", query);
		else
			json::value_from(user_ids, data["user_ids"]);
		char str[20];
		auto[end, _] = std::to_chars(std::begin(str), std::end(str), nonce);
		data.emplace("nonce", json::string_view(str, end - str));
		return json::serialize(obj);
	}());
	/* Destroy the entry at the end, even if the generator isn't consumed fully */
	volatile const auto cleanup = [this, nonce] {
		struct _ {
			implementation* self;
			std::uint64_t nonce;
			~_() try {
				asio::defer(self->m_ws_strand, [s = self, n = nonce] { s->m_rgm_entries.erase(n); });
			} catch (...) {}
		}; return _{ this, nonce };
	}();
	/* An awaitable to suspend the coroutine and save it at the entry map; to be resumed when chunks start arriving */
	const auto suspend_awaitable = [this, nonce] {
		struct _ {
			implementation* self;
			std::uint64_t nonce;

			bool await_ready() const noexcept { return false; }
			void await_suspend(std::coroutine_handle<> h) const {
				auto& entry = self->m_rgm_entries[nonce];
				if (auto ex = entry.except)
					std::rethrow_exception(ex);
				entry.started_at = chrono::system_clock::now();
				entry.coro = h;
			}
			auto& await_resume() const {
				auto& entry = self->m_rgm_entries[nonce];
				if (auto ex = entry.except)
					std::rethrow_exception(ex);
				entry.coro = nullptr;
				return entry.chunks;
			}
		}; return _{ this, nonce };
	}();
	/* Wait for the first chunk */
	auto& chunks = co_await suspend_awaitable;
	for (std::size_t chunk_index = 0, chunk_count = chunks.front().GetChunkCount(); chunk_index < chunk_count; ++chunk_index) {
		auto it = chunks.begin();
		/* If we're out of chunks, wait for more to arrive */
		if (it == chunks.end()) {
			co_await suspend_awaitable;
			it = chunks.begin();
		}
		/* Validate chunk count and index */
		if (auto index = it->GetChunkIndex(); it->GetChunkCount() != chunk_count || index != chunk_index || index >= chunk_count)
			throw xGatewayEventError();
		/* Yield all members */
		for (auto& member : it->GetMembers())
			co_yield member;
		/* Remove the chunk that was just processed from the queue */
		co_await ResumeOnWebSocketStrand();
		chunks.pop_front();
	}
}