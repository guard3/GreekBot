#ifndef GREEKBOT_UTILS_H
#define GREEKBOT_UTILS_H
#include <random>
#include <string>
#include <concepts>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <fmt/core.h>

class xNumberFormatError : public std::invalid_argument {
public:
	explicit xNumberFormatError(const char* str) : std::invalid_argument(str) {}
	explicit xNumberFormatError(const std::string& str) : xNumberFormatError(str.c_str()) {}
};

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

	static std::vector<uint8_t> base64_decode(const char*, size_t);
	static std::string percent_encode(const char*, size_t);
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
	template<std::integral Result = int, typename String>
	static Result ParseInt(String &&str) {
		Result result;
		/* Resolve string argument */
		const char* s;
		if constexpr (requires { { str.c_str() } -> std::convertible_to<const char*>; })
			s = static_cast<const char*>(str.c_str());
		else
			s = str;
		/* Clear errno */
		errno = 0;
		/* Parse string */
		char* end;
		if constexpr (std::same_as<Result, long long>)
			result = strtoll(s, &end, 10);
		else if constexpr (std::same_as<Result, unsigned long long>)
			result = strtoull(s, &end, 10);
		else if constexpr (std::unsigned_integral<Result>) {
			auto r = strtoul(s, &end, 10);
			result = static_cast<Result>(r);
			if (result != r) goto LABEL_THROW_RANGE;
		}
		else {
			auto r = strtol(s, &end, 10);
			result = static_cast<Result>(r);
			if (result != r) goto LABEL_THROW_RANGE;
		}
		/* Check for range errors */
		if (errno == ERANGE) {
			LABEL_THROW_RANGE:
			throw xNumberFormatError("Parsed number is out of range");
		}
		/* Check for success */
		if (*end)
			throw xNumberFormatError("String can't be parsed into an integer");
		return result;
	}
	/* Base64 encode/decode */
	static std::string Base64Encode(const void* data, size_t size);
	static std::vector<uint8_t> Base64Decode(const std::string& str) {
		return base64_decode(str.data(), str.length());
	}
	static std::vector<uint8_t> Base64Decode(const char* str) {
		return base64_decode(str, strlen(str));
	}
	/* Percent encoding */
	static std::string PercentEncode(const char*        str) { return percent_encode(str,         strlen(str));  }
	static std::string PercentEncode(const std::string& str) { return percent_encode(str.c_str(), str.length()); }
	/* Parse ISO8601 timestamp */
	static std::chrono::sys_seconds ParseTimestamp(const std::string&);
	/* Resolving the OS we're running on */
	static const char* GetOS();
};

template<std::integral I>       struct cUtils::d<I> { typedef std::uniform_int_distribution<I>  type; };
template<std::floating_point F> struct cUtils::d<F> { typedef std::uniform_real_distribution<F> type; };
#endif //GREEKBOT_UTILS_H