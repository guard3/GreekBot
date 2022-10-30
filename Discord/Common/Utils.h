#pragma once
#ifndef GREEKBOT_UTILS_H
#define GREEKBOT_UTILS_H
#include <random>
#include <string>
#include <concepts>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#if 0
namespace discord::detail {
	/* Generic random distribution types */
	namespace distribution_detail {
		template<typename T>            struct d {};
		template<std::integral I>       struct d<I> { typedef std::uniform_int_distribution<I>  type; };
		template<std::floating_point F> struct d<F> { typedef std::uniform_real_distribution<F> type; };
	}
	template<typename T> using distribution = typename distribution_detail::d<T>::type;
	template<typename T> using range        = typename distribution<T>::param_type;
}
#endif
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
	/* Non templated static functions */
	static void print(FILE*, const char*, char, const char*, ...);
	static std::string format(const char*, ...);

	template<typename T>
	static auto resolve_va_arg(T&& arg) {
		if constexpr (requires { {arg.c_str()} -> std::convertible_to<const char*>; })
			return static_cast<const char*>(arg.c_str());
		else
			return arg;
	}

	static std::vector<uint8_t> base64_decode(const char*, size_t);
	/* Private constructor */
	cUtils() = default;
public:
	cUtils(const cUtils&) = delete;
	cUtils& operator=(const cUtils&) = delete;
	/* Random functions */
	template<typename T1, typename T2, typename R = std::common_type_t<T1, T2>>
	static R Random(T1 a, T2 b) {
		//using namespace discord::detail;
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
	static void PrintErr(Args&&... args) {
		//using namespace discord::detail;
		print(stderr, "[ERR] ", nl, resolve_va_arg(std::forward<Args>(args))...);
	}
	template<char nl = '\n', typename... Args>
	static void PrintLog(Args&&... args) {
		//using namespace discord::detail;
		print(stdout, "[LOG] ", nl, resolve_va_arg(std::forward<Args>(args))...);
	}
	template<char nl = '\n', typename... Args>
	static void PrintMsg(Args&&... args) {
		//using namespace discord::detail;
		print(stdout, "[MSG] ", nl, resolve_va_arg(std::forward<Args>(args))...);
	}
	/* C style formatting for std::string */
	template<typename... Args>
	static std::string Format(Args&&... args) {
		//using namespace discord::detail;
		return format(resolve_va_arg(std::forward<Args>(args))...);
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
	/* Resolving the OS we're running on */
	static const char* GetOS();
};

template<std::integral I>       struct cUtils::d<I> { typedef std::uniform_int_distribution<I>  type; };
template<std::floating_point F> struct cUtils::d<F> { typedef std::uniform_real_distribution<F> type; };
#endif //GREEKBOT_UTILS_H