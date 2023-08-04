#ifndef GREEKBOT_UTILS_H
#define GREEKBOT_UTILS_H
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <concepts>
#include <stdexcept>
#include <charconv>
#include <chrono>
#include <vector>
#include <cstdint>
#include <fmt/core.h>

/* A helper class with various useful functions */
class cUtils final {
private:
	/* Random generators */
	static std::mt19937    ms_gen;
	static std::mt19937_64 ms_gen64;
	/* Distribution types */
	template<typename T> struct d;
	template<typename T> using distribution = typename d<T>::type;
	template<typename T> using range        = typename distribution<T>::param_type;
	/* Private constructor */
	cUtils() = default;
public:
	cUtils(const cUtils&) = delete;
	cUtils& operator=(const cUtils&) = delete;
	/* Random functions */
	template<typename T1, typename T2, typename R = std::common_type_t<T1, T2>>
	static R Random(T1 a, T2 b) {
		/* Static uniform distribution */
		static distribution<R> dist;
		/* Generate random number */
		if constexpr(sizeof(R) < 8)
			return dist(ms_gen,   range<R>(a, b));
		else
			return dist(ms_gen64, range<R>(a, b));
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
		auto result = std::from_chars(str.begin(), str.end(), value);
		if (result.ec == std::errc{} && result.ptr == str.end())
			return value;
		if (result.ec == std::errc::result_out_of_range)
			throw std::out_of_range("Parsed integer is out of range");
		throw std::invalid_argument("Input string can't be parsed into an integer");
	}
	/* Base64 encode/decode */
	static std::string Base64Encode(const void* data, size_t size);
	static std::vector<uint8_t> Base64Decode(std::string_view);
	/* Percent encoding */
	static std::string PercentEncode(std::string_view);
	/* Parse ISO8601 timestamp */
	static std::chrono::sys_seconds ParseTimestamp(std::string_view);
	/* Resolving the OS we're running on */
	static const char* GetOS();
};

template<std::integral I>       struct cUtils::d<I> { typedef std::uniform_int_distribution<I>  type; };
template<std::floating_point F> struct cUtils::d<F> { typedef std::uniform_real_distribution<F> type; };
#endif //GREEKBOT_UTILS_H