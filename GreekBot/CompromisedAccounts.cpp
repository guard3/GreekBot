#include "CDN.h"
#include "DBCompromisedAccounts.h"
#include "GreekBot.h"
#include "Utils.h"

#include <cmath>

using namespace std::chrono;

// In order to flag a user as compromised we need to get the latest metrics on their message activity.
//
// The idea is simple: If the same message is detected in enough different channels at a fast enough rate, then there
// is a high probability this user is compromised and needs to be flagged as such.
//
// We need 2 metrics to determine this:
// - The number of distinct channels the same message appeared in (duplicates in the same channel are not a valid pattern)
// - The average message send rate of those messages (typically compromised accounts do this as fast as possible)
//
// To convert the above into a probability, let's say that when a message:
// - is sent in 10 different channels
// - at a rate of 0.6 messages/s
// this should produce a probability of 0.98
//
// So we get the following constants:
static constexpr double TARGET_PROBABILITY = 0.98;
static constexpr int    CHANNEL_THRESHOLD  = 10;
static constexpr double RATE_THRESHOLD     = 0.6;
//
// To produce the final probability, we'll feed the above to a sigmoid function

// Unfortunately, constexpr math is available from C++26, which is not the target standard version *yet*
// so we need a workaround for that...
#if __cpp_lib_constexpr_cmath >= 202306L
#define GREEKBOT_HAS_CONSTEXPR_MATH
#define GREEKBOT_CONSTEXPR constexpr
#else
#define GREEKBOT_CONSTEXPR const
#endif

double
GREEKBOT_CONSTEXPR static logit(double p) noexcept {
	return std::log(p / (1.0 - p));
}

double
GREEKBOT_CONSTEXPR static weight(double p1, double x1, double p2, double x2) noexcept {
	return (logit(p2) - logit(p1)) / (x2 - x1);
}

double
GREEKBOT_CONSTEXPR static sigmoid(double x) noexcept {
	return 1.0 / (1.0 + std::exp(-x));
}

double
static spam_probability(std::int64_t num_channels, milliseconds period) noexcept {
	// If the period is 0 we're likely dealing with the first unique appearance of a message; definitely not spam
	if (period == 0ms)
		return 0.0;

	[[maybe_unused]] auto calculate_logit = [] { return logit(TARGET_PROBABILITY); };
	[[maybe_unused]] auto calculate_wc = [] { return weight(0.7, CHANNEL_THRESHOLD - 2, 0.98, CHANNEL_THRESHOLD); };
	[[maybe_unused]] auto calculate_wr = [] { return weight(0.98, RATE_THRESHOLD, 0.99, 1.0); };

	// Calculate weights
#ifdef GREEKBOT_HAS_CONSTEXPR_MATH
	constexpr double LOGIT = calculate_logit();
	constexpr double wC = calculate_wc();
	constexpr double wR = calculate_wr();
#elif 0 // Conditionally enable this to update constants manually...
	static const double LOGIT = calculate_logit();
	static const double wC = calculate_wc();
	static const double wR = calculate_wr();

	cUtils::PrintDbg("LOGIT: {}, wC: {}, wR: {}", LOGIT, wC, wR);
#else
	constexpr double LOGIT = 3.8918202981106256;
	constexpr double wC = 1.522261218861711;
	constexpr double wR = 1.7582488800599083;
#endif

	double C = static_cast<double>(num_channels);
	double R = C / duration_cast<duration<double>>(period).count();

	double z = LOGIT
	         + wC * (C - CHANNEL_THRESHOLD)
	         + wR * (R - RATE_THRESHOLD);

	return sigmoid(z);
}

cTask<>
cGreekBot::process_compro_new_message(const cMessage& msg, const cPartialMember& member) {
	auto [num_channels, period] = co_await cCompromisedAccountsDAO(co_await cTransaction::New()).GetMetrics(msg);

	double p = spam_probability(num_channels, period);

	// If the probability is determined to be above 99%, send an alert
	// Ideally this should autoban the compromised user but for now we'll send alerts as a test run
	if (p < 0.99)
		co_return;

	cPartialMessage response;
	auto& embed = response.EmplaceEmbeds().emplace_back();

	auto& author = msg.GetAuthor();
	embed.SetColor(LMG_COLOR_RED);
	embed.EmplaceAuthor(author.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(author));
	embed.SetDescription("⚠️ Potentially compromised account");
	embed.SetFields({
		{ "User ID", std::format("`{}`", author.GetId()) },
		{ "Message ID", std::format("`{}`", msg.GetId()) },
		{ "Probability", std::format("{:.5}%", p * 100.0), true },
		{ "Messages", std::to_string(num_channels), true },
		{ "Time span", std::format("{}", duration_cast<duration<double>>(period)), true }
	});
	embed.SetTimestamp(msg.GetTimestamp());
	response.SetComponents({
		cActionRow{
			// TODO: Add a quick ban button
			cButton{
				eButtonStyle::Secondary,
				std::format("DLT#{}", GetUser().GetId()),
				"Dismiss"
			}
		}
	});

	co_await CreateMessage(LMG_CHANNEL_COMPROMISED, response
		.SetContent(std::format("Staff wake up! https://discord.com/channels/{}/{}/{}", LMG_GUILD_ID, msg.GetChannelId(), msg.GetId()))
	);
}
