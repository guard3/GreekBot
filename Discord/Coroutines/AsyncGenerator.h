#ifndef DISCORD_ASYNCGENERATOR_H
#define DISCORD_ASYNCGENERATOR_H
#include "Task.h"
/* ========== An exception thrown when a generator is empty ========================================================= */
class xGeneratorError : public std::exception {
public:
	const char* what() const noexcept override = 0;
};
/* ========== Generators can only accept non reference non void types =============================================== */
template<typename T>
concept iGeneratorValue = !std::is_void_v<T> && !std::is_reference_v<T>;
/* ========== Forward declaration of the promise type =============================================================== */
namespace detail {
	template<iGeneratorValue> struct async_gen_promise_base;
	template<iGeneratorValue> struct async_gen_promise_type;
}
/* ========== An asynchronous generator that allows both co_yield and co_await in its body ========================== */
template<iGeneratorValue T>
class cAsyncGenerator {
public:
	using promise_type = detail::async_gen_promise_type<T>;
	class iterator;
	/* Move-only constructor */
	cAsyncGenerator(const cAsyncGenerator&) = delete;
	cAsyncGenerator(cAsyncGenerator&& o) noexcept : m_coro(std::exchange(o.m_coro, {})) {}
	~cAsyncGenerator() { if (m_coro) m_coro.destroy(); }
	/* Move-only assignment */
	cAsyncGenerator& operator=(cAsyncGenerator o) noexcept {
		swap(m_coro, o.m_coro);
		return *this;
	}
	/* Iterators */
	cTask<iterator> begin();
	std::default_sentinel_t end() noexcept { return {}; }
	/*A for_each style helper function */
	template<std::invocable<T&> Fn>
	cTask<> ForEach(Fn&& fn);
private:
	std::coroutine_handle<promise_type> m_coro;
	/* Private constructor */
	explicit cAsyncGenerator(std::coroutine_handle<promise_type> coro) noexcept : m_coro(coro) {}
	friend auto detail::async_gen_promise_base<T>::get_return_object() noexcept;
};
/* ========== The base promise with partial functionality regardless if the yielded value is const or not =========== */
template<iGeneratorValue T>
struct detail::async_gen_promise_base {
	T*                      value{}; // A pointer to the yielded value, or null if none exists
	std::exception_ptr      except;  // An exception caught inside the coroutine body
	std::coroutine_handle<> caller;  // The coroutine handle of the caller, if it exists
	/* The necessary function definitions for this promise */
	auto get_return_object() noexcept { return cAsyncGenerator{ std::coroutine_handle<async_gen_promise_type<T>>::from_promise(*static_cast<async_gen_promise_type<T>*>(this)) }; }
	auto initial_suspend() noexcept { return std::suspend_always{}; }
	void return_void() noexcept {}
	void unhandled_exception() { except = std::current_exception(); }
	/* The awaitable functions to make the promise suspend the generator coroutine and return control to the caller */
	bool await_ready() noexcept { return false; }
	std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept {
		return caller ? std::exchange(caller, {}) : std::noop_coroutine();
	}
	void await_resume() noexcept {}
};
/* ========== The promise type for non-const yielded values ========================================================= */
template<iGeneratorValue T>
struct detail::async_gen_promise_type final : async_gen_promise_base<T> {
private:
	alignas(alignof(T)) std::byte buff[sizeof(T)]; // A buffer to hold a copy of the yielded value if a simple reference isn't possible

	T* destroy_value() noexcept {
		auto pBuff = reinterpret_cast<T*>(buff);
		if (this->value == pBuff)
			std::destroy_at(pBuff);
		return pBuff;
	}
	void clear_value() noexcept {
		destroy_value();
		this->value = nullptr;
	}
	void set_value(T& t) noexcept {
		destroy_value();
		this->value = std::addressof(t);
	}
	void set_value(const T& t) noexcept(std::is_nothrow_copy_constructible_v<T>) {
		this->value = std::construct_at(destroy_value(), t);
	}

public:
	~async_gen_promise_type() { destroy_value(); }

	template<typename U>
	U&& await_transform(U&& u) {
		clear_value();
		return std::forward<U>(u);
	}
	async_gen_promise_type& yield_value(T& t) noexcept {
		set_value(t);
		return *this;
	}
	async_gen_promise_type& yield_value(T&& t) noexcept {
		set_value(t);
		return *this;
	}
	async_gen_promise_type& yield_value(const T& t) noexcept(std::is_nothrow_copy_constructible_v<T>) {
		set_value(t);
		return *this;
	}
	async_gen_promise_type& final_suspend() noexcept {
		clear_value();
		return *this;
	}
};
/* ========== The promise type for const yielded values ============================================================= */
template<iGeneratorValue T>
struct detail::async_gen_promise_type<const T> final : async_gen_promise_base<const T> {
	template<typename U>
	U&& await_transform(U&& u) noexcept {
		this->value = nullptr;
		return std::forward<U>(u);
	}
	async_gen_promise_type& yield_value(const T& t) noexcept {
		this->value = std::addressof(t);
		return *this;
	}
	async_gen_promise_type& final_suspend() noexcept {
		this->value = nullptr;
		return *this;
	}
};

template<iGeneratorValue T>
class cAsyncGenerator<T>::iterator final {
private:
	std::coroutine_handle<promise_type> m_coro;
	/* Private constructor to allow an iterator to be created only through begin() */
	explicit iterator(std::coroutine_handle<promise_type> coro) noexcept : m_coro(coro) {}
	friend cTask<iterator> cAsyncGenerator::begin();
	/* Awaitable functions to save the caller in the coroutine promise and resume later */
	bool await_ready() noexcept {
		if (!m_coro)
			return true;
		m_coro.resume();
		return m_coro.done() || m_coro.promise().value;
	}
	void await_suspend(std::coroutine_handle<> h) noexcept {
		m_coro.promise().caller = h;
	}
	void await_resume() {
		if (m_coro && m_coro.done()) {
			auto ex = m_coro.promise().except;
			m_coro.destroy();
			m_coro = nullptr;
			if (ex)
				std::rethrow_exception(ex);
		}
	}

public:
	/* Move-only constructor */
	iterator(const iterator&) = delete;
	iterator(iterator&& o) noexcept : m_coro(std::exchange(o.m_coro, {})) {}
	~iterator() { if (m_coro) m_coro.destroy(); }
	/* Move-only assignment */
	iterator& operator=(iterator o) noexcept {
		swap(m_coro, o.m_coro);
		return *this;
	}
	/* Operators to access the currently yielded value */
	T* operator->() const noexcept {
		return m_coro ? m_coro.promise().value : nullptr;
	}
	T& operator*() const {
		if (!m_coro) {
			class _ : public xGeneratorError {
			public:
				const char* what() const noexcept override { return "The asynchronous generator is empty"; }
			};
			throw _();
		}
		return *m_coro.promise().value;
	}
	/* Resume the generator coroutine to the next suspension point */
	cTask<> operator++()    { co_await *this; }
	cTask<> operator++(int) { co_await *this; }
	/* Comparison with the end() sentinel */
	bool operator==(std::default_sentinel_t) const noexcept {
		return !m_coro || m_coro.done();
	}
};
/* ========== Comparison with the end() sentinel ==================================================================== */
template<iGeneratorValue T>
inline bool operator==(std::default_sentinel_t sentinel, const typename cAsyncGenerator<T>::iterator& it) noexcept {
	return it == sentinel;
}
/* ========== begin() implementation ================================================================================ */
template<iGeneratorValue T>
inline cTask<typename cAsyncGenerator<T>::iterator> cAsyncGenerator<T>::begin() {
	/* Transfer coroutine ownership to the result iterator */
	iterator it{ std::exchange(m_coro, {}) };
	/* Resume the coroutine to the first yield suspension point before returning */
	co_await it;
	co_return it;
}
/* ========== A for_each style helper function ====================================================================== */
template<iGeneratorValue T>
template<std::invocable<T&> Fn>
inline cTask<> cAsyncGenerator<T>::ForEach(Fn&& fn) {
	for (auto it = co_await begin(); it != end(); co_await ++it)
		co_await fn(*it);
}
#endif /* DISCORD_ASYNCGENERATOR_H */