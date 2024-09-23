#include "Bot.h"
#include "Utils.h"
#include <algorithm>
#include <boost/json.hpp>

namespace rng = std::ranges;
namespace json = boost::json;

cTask<>
cBot::delete_message(const cSnowflake& channel_id, const cSnowflake& msg_id, std::span<const cHttpField> fields) {
	char begin[128];
	const char* end = fmt::format_to(begin, "/channels/{}/messages/{}", channel_id, msg_id);
	co_await DiscordDelete({ begin, end }, fields);
}

cTask<>
cBot::DeleteMessage(const cSnowflake& channel_id, const cSnowflake& msg_id, std::string_view reason) {
	std::optional<cHttpField> opt;
	co_await delete_message(channel_id, msg_id, reason.empty() ? std::span<cHttpField>() : std::span(&opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1));
}

cTask<>
cBot::DeleteMessages(crefChannel channel, std::span<const cSnowflake> msg_ids, std::string_view reason) {
	/* Create the audit log reason field, if provided */
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span(&opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1);
	/* Formulate the bulk-delete endpoint path */
	char bulk_delete_begin[64];
	const char* bulk_delete_end = fmt::format_to(bulk_delete_begin, "/channels/{}/messages/bulk-delete", channel.GetId());
	/* Start deleting messages in batched of at most 100 messages */
	const cSnowflake* ids[100];
	for (std::size_t ids_size; !msg_ids.empty(); msg_ids = msg_ids.subspan(ids_size)) {
		ids_size = std::min<std::size_t>(msg_ids.size(), 100); // The array size to use in this iteration
		const auto ids_end = ids + ids_size;                   // A past-the-end pointer to the ids array
		/* And now the fun begins!
		 * We add pointers to the first batch of snowflakes, partitioning them based on their post date. That's needed,
		 * because the bulk-delete endpoint can only delete messages not older than 2 weeks. At the end, 'it' will be
		 * pointing to the first older-than-2-weeks ids like so:
		 * [----- newer messages -----][----- older messages ----- ]
		 * ^~~~ ids                    ^~~~ it           ids_end ~~~^
		 */
		auto it = [out_begin = ids, out_end = ids_end, msg_ids]() mutable {
			using namespace std::chrono;
			const auto now = system_clock::now();
			for (const cSnowflake& id : msg_ids.subspan(0, out_end - out_begin)) {
				if (ceil<days>(now - id.GetTimestamp()) < days(14))
					*out_begin++ = &id;
				else
					*--out_end = &id;
			}
			return out_end;
		}();
		if (std::span newer(ids, it); newer.size() >= 2) {
			/* If we have at least 2 messages newer than 2 weeks, bulk delete... */
			json::object obj;
			auto& array = obj["messages"].emplace_array();
			array.reserve(newer.size());
			for (const cSnowflake* pId : newer)
				array.emplace_back(pId->ToString());
			co_await DiscordPost({ bulk_delete_begin, bulk_delete_end }, obj, fields);
		} else {
			/* ...if not, move the iterator to point to the beginning of all messages */
			it = ids;
		}
		/* It's best to sort older messages from newest to oldest... */
		std::span older(it, ids_end);
		rng::sort(older, [](const cSnowflake* lhs, const cSnowflake* rhs) { return *lhs > *rhs; });
		/* ...because they need to be deleted one by one */
		for (const cSnowflake* pId : older)
			co_await delete_message(channel.GetId(), *pId, fields);
	}
}

cAsyncGenerator<cMessage>
cBot::get_channel_messages(std::string path) {
	const auto json = co_await DiscordGet(path);
	for (auto& v : json.as_array())
		co_yield cMessage{ v };
}

cAsyncGenerator<cMessage>
cBot::GetChannelMessages(crefChannel channel, std::size_t limit) {
	if (limit == 0) {
		return []() -> cAsyncGenerator<cMessage> { co_return; }();
	} else if (limit > 100) {
		return [](cBot& self, cSnowflake channel_id, std::size_t limit) -> cAsyncGenerator<cMessage> {
			/* First, return the first 100 messages and save the last message id */
			std::uint64_t last_id = 0;
			std::size_t count = 0;
			co_for (cMessage& msg, self.get_channel_messages(fmt::format("/channels/{}/messages?limit={}", channel_id, 100))) {
				last_id = msg.GetId().ToInt();
				++count;
				co_yield msg;
			}
			/* If we receive fewer messages than requested, we're done */
			if (count < 100)
				co_return;
			/* Request all remaining messages sent before the last received message */
			co_for (cMessage& msg, self.GetChannelMessagesBefore(channel_id, cSnowflake(last_id), limit - 100))
				co_yield msg;
		}(*this, channel.GetId(), limit);
	} else try {
		return get_channel_messages(fmt::format("/channels/{}/messages?limit={}", channel.GetId(), limit));
	} catch (...) {
		return [](std::exception_ptr ex) -> cAsyncGenerator<cMessage> {
			std::rethrow_exception(ex);
			co_return;
		}(std::current_exception());
	}
}

cAsyncGenerator<cMessage>
cBot::GetChannelMessagesBefore(crefChannel channel, crefMessage before_this_message, std::size_t limit) {
	if (limit == 0) {
		return []() -> cAsyncGenerator<cMessage> { co_return; }();
	} else if (limit > 100) {
		return [](cBot& self, std::uint64_t channel_id, std::uint64_t before_this_message, std::size_t limit) -> cAsyncGenerator<cMessage> {
			std::size_t count;
			do {
				/* Fetch a batch of at most 100 messages */
				count = 0;
				limit -= 100;
				co_for (cMessage& msg, self.get_channel_messages(fmt::format("/channels/{}/messages?before={}&limit={}", channel_id, before_this_message, 100))) {
					++count;
					before_this_message = msg.GetId().ToInt();
					co_yield msg;
				}
				/* If we receive less than the amount requested, we're done */
				if (count < 100)
					co_return;
			} while (limit > 100);
			/* The limit is at most 100, so get the final batch of messages */
			co_for (cMessage& msg, self.get_channel_messages(fmt::format("/channels/{}/messages?before={}&limit={}", channel_id, before_this_message, limit)))
				co_yield msg;
		}(*this, channel.GetId().ToInt(), before_this_message.GetId().ToInt(), limit);
	} else try {
		return get_channel_messages(fmt::format("/channels/{}/messages?before={}&limit={}", channel.GetId(), before_this_message.GetId(), limit));
	} catch (...) {
		return [](std::exception_ptr ex) -> cAsyncGenerator<cMessage> {
			std::rethrow_exception(ex);
			co_return;
		}(std::current_exception());
	}
}