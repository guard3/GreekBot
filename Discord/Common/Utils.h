#ifndef GREEKBOT_UTILS_H
#define GREEKBOT_UTILS_H
#include <charconv>
#include <chrono>
#include <concepts>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <fmt/format.h>

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
	static void PrintErr(fmt::format_string<Args...> format, Args&&... args) {
		fmt::print(stderr, "[ERR] {}{}", fmt::format(format, std::forward<Args>(args)...), nl);
	}
	template<char nl = '\n', typename... Args>
	static void PrintLog(fmt::format_string<Args...> format, Args&&... args) {
		fmt::print(stdout, "[LOG] {}{}", fmt::format(format, std::forward<Args>(args)...), nl);
	}
	template<char nl = '\n', typename... Args>
	static void PrintMsg(fmt::format_string<Args...> format, Args&&... args) {
		fmt::print(stdout, "[MSG] {}{}", fmt::format(format, std::forward<Args>(args)...), nl);
	}
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
	static uint32_t CRC32(uint32_t, std::string_view) noexcept;
	/* Base64 encode/decode */
	static std::string Base64Encode(const void* data, size_t size);
	static std::vector<uint8_t> Base64Decode(std::string_view);
	/* Percent encoding */
	static std::string PercentEncode(std::string_view);
	/* Parse ISO8601 timestamp; basic or extended format */
	static std::chrono::sys_time<std::chrono::milliseconds> ParseISOTimestamp(std::string_view);
	/* Resolving the OS we're running on */
	static std::string_view GetOS();
};
#endif //GREEKBOT_UTILS_H