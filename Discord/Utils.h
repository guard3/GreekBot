#ifndef DISCORD_UTILS_H
#define DISCORD_UTILS_H
#include <charconv>
#include <chrono>
#include <concepts>
#include <filesystem>
#include <format>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/* A helper class with various useful functions */
class cUtils final {
private:
	/* Random generators */
	static std::mt19937    ms_gen;
	static std::mt19937_64 ms_gen64;
	/* Distribution types */
	template<typename T>
	using distribution = std::conditional_t<std::is_integral_v<T>,
	                                        std::uniform_int_distribution<std::conditional_t<(sizeof(T) < sizeof(short)), int, T>>,
	                                        std::uniform_real_distribution<T>>;
	/* Print helpers */
	static void print_err(std::string, char) noexcept;
	static void print_log(std::string, char) noexcept;
	static void print_msg(std::string, char) noexcept;
	static void print_dbg(std::string, char) noexcept;
public:
	cUtils() = delete;
	/* Random functions */
	template<typename A, typename B> requires(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>)
	static auto Random(A a, B b) {
		/* Set the return type as the common type of A and B */
		using return_t = std::common_type_t<A, B>;
		/* Create a static uniform distribution for the common type */
		static distribution<return_t> dist;
		/* Generate the random number */
		if constexpr(sizeof(return_t) < 8)
			return static_cast<return_t>(dist(ms_gen,   typename distribution<return_t>::param_type{ a, b }));
		else
			return static_cast<return_t>(dist(ms_gen64, typename distribution<return_t>::param_type{ a, b }));
	}
	/* Logger functions */
	template<char nl = '\n', typename... Args>
	static void PrintErr(std::format_string<Args...> format, Args&&... args) noexcept try {
		print_err(std::format(format, std::forward<Args>(args)...), nl);
	} catch (...) {
		print_err(std::string(), nl);
	}
	template<char nl = '\n', typename... Args>
	static void PrintLog(std::format_string<Args...> format, Args&&... args) noexcept try {
		print_log(std::format(format, std::forward<Args>(args)...), nl);
	} catch (...) {
		print_log(std::string(), nl);
	}
	template<char nl = '\n', typename... Args>
	static void PrintMsg(std::format_string<Args...> format, Args&&... args) noexcept try {
		print_msg(std::format(format, std::forward<Args>(args)...), nl);
	} catch (...) {
		print_msg(std::string(), nl);
	}
	template<char nl = '\n', typename... Args>
	static void PrintDbg(std::format_string<Args...> format, Args&&... args) noexcept
#ifdef DEBUG
	try {
		print_dbg(std::format(format, std::forward<Args>(args)...), nl);
	} catch (...) {
		print_dbg(std::string(), nl);
	}
#else
	{}
#endif
	/* Converting a string to int */
	template<std::integral T = int>
	static T ParseInt(std::string_view str) {
		T value;
		const char* begin = str.data();
		const char* end   = str.data() + str.size();
		auto[ptr, err] = std::from_chars(begin, end, value);
		if (err == std::errc{} && ptr == end)
			return value;
		if (err == std::errc::result_out_of_range)
			throw std::out_of_range("Parsed integer is out of range");
		throw std::invalid_argument("Input string can't be parsed into an integer");
	}
	/* Calculate crc32 checksums of strings */
	static std::uint32_t CRC32(std::uint32_t, std::string_view) noexcept;
	/* Base64 encode/decode */
	static std::string Base64Encode(const void* data, size_t size);
	static std::vector<uint8_t> Base64Decode(std::string_view);
	/* Percent encoding */
	static std::string PercentEncode(std::string_view);
	/* Parse ISO8601 timestamp; basic or extended format */
	static std::chrono::sys_time<std::chrono::milliseconds> ParseISOTimestamp(std::string_view);
	template<typename Duration>
	static std::string FormatISOTimestamp(std::chrono::sys_time<Duration> timepoint) {
		using namespace std::chrono;
		const auto tp_days = floor<days>(timepoint);
		const year_month_day ymd{ tp_days };
		const hh_mm_ss hms{ timepoint - tp_days };
		if constexpr (std::floating_point<typename Duration::rep> || decltype(hms)::fractional_width > 6) {
			return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:09}Z", (int)ymd.year(), (unsigned)ymd.month(), (unsigned)ymd.day(), hms.hours().count(), hms.minutes().count(), hms.seconds().count(), floor<nanoseconds>(hms.subseconds()).count());
		} else {
			return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:06}Z", (int)ymd.year(), (unsigned)ymd.month(), (unsigned)ymd.day(), hms.hours().count(), hms.minutes().count(), hms.seconds().count(), floor<microseconds>(hms.subseconds()).count());
		}
	}
	/* Resolving the OS we're running on */
	static std::string_view GetOS() noexcept;
	/* Get a fully qualified path to the running executable */
	static std::filesystem::path GetExecutablePath();
};
#endif /* DISCORD_UTILS_H */