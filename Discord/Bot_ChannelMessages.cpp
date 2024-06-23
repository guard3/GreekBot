#include "Bot.h"
#include <boost/json.hpp>

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