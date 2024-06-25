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
		/* Save pointers to the first 100 snowflakes, partitioning them based on their post date;
		 * Messages older than 2 weeks are saved at the end of the array, with 'it' pointing to the first such message */
		ids_size = std::min<std::size_t>(msg_ids.size(), 100);
		auto it = [out_begin = ids, out_end = ids + ids_size, msg_ids]() mutable {
			using namespace std::chrono;
			const auto num = out_end - out_begin;
			const auto now = floor<days>(system_clock::now());
			for (std::size_t i = 0; i < num; ++i) {
				if (const auto pId = &msg_ids[i]; now - ceil<days>(pId->GetTimestamp()) < days(14))
					*out_begin++ = pId;
				else
					*--out_end = pId;
			}
			return out_end;
		}();
		if (const auto num = it - ids; num > 1) {
			/* If the messages that are not older than 2 weeks are at least 2, bulk delete */
			json::object obj;
			auto& array = obj["messages"].emplace_array();
			array.reserve(num);
			for (std::size_t i = 0; i < num; ++i)
				array.emplace_back(ids[i]->ToString());
			co_await DiscordPost({ bulk_delete_begin, bulk_delete_end }, obj, fields);
		} else if (num == 1) {
			/* If there's only 1 message, move the iterator to point to the beginning of all messages */
			it = ids;
		}
		/* All messages older than 2 weeks need to be deleted one by one... */
		for (const auto end = ids + ids_size; it != end; ++it)
			co_await delete_message(channel.GetId(), **it, fields);
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