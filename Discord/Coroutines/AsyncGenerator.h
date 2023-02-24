#ifndef GREEKBOT_ASYNCGENERATOR_H
#define GREEKBOT_ASYNCGENERATOR_H
#include "Task.h"

template<typename T>
class cAsyncGenerator {
public:
	class promise_type;

	cAsyncGenerator(const cAsyncGenerator&) = delete;
	cAsyncGenerator(cAsyncGenerator&& o) noexcept : m_handle(o.m_handle) { o.m_handle = nullptr; }
	~cAsyncGenerator() { if (m_handle) m_handle.destroy(); }

	cAsyncGenerator& operator=(const cAsyncGenerator&) = delete;
	cAsyncGenerator& operator=(cAsyncGenerator&& o) noexcept {
		~cAsyncGenerator();
		return *new(this) cAsyncGenerator(std::forward<cAsyncGenerator>(o));
	}

	cTask<bool> HasValue();
	cTask<T> Next();

	cTask<bool> operator co_await() { return HasValue(); }
	cTask<T> operator()() { return Next(); }

private:
	std::coroutine_handle<promise_type> m_handle;

	cAsyncGenerator(std::coroutine_handle<promise_type> h) : m_handle(h) {}
};

template<typename T>
class cAsyncGenerator<T>::promise_type {
private:
	std::exception_ptr m_except;
	std::optional<T> m_value;
	std::coroutine_handle<> m_caller;
	std::coroutine_handle<promise_type> m_coro = std::coroutine_handle<promise_type>::from_promise(*this);
	bool m_ready = false;

public:
	/* An awaitable for final_suspend and yield_value that always returns control to the caller */
	struct suspend_awaitable : std::suspend_always {
		std::coroutine_handle<> caller;
		std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept { return caller; }
	};
	/* The necessary function definitions for this promise */
	cAsyncGenerator get_return_object() noexcept { return m_coro; }
	std::suspend_always initial_suspend() noexcept { return {}; }
	suspend_awaitable final_suspend() noexcept {
		m_ready = true;
		return {{}, m_caller};
	}
	template<typename U = T>
	suspend_awaitable yield_value(U&& u) {
		m_ready = true;
		m_value = std::forward<U>(u);
		return {{}, m_caller};
	}
	void return_void() {}
	void unhandled_exception() { m_except = std::current_exception(); }
	/* The awaitable functions */
	bool await_ready() noexcept { return m_ready; }
	std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept {
		m_caller = h;
		return m_coro;
	}
	bool await_resume() {
		if (m_except) std::rethrow_exception(m_except);
		return !m_coro.done();
	}
	/* A function that sets the promise at an empty state and returns the pending value */
	T publish() {
		m_ready = false;
		return std::move(*m_value);
	}
};

template<typename T>
inline cTask<bool> cAsyncGenerator<T>::HasValue() {
	/* Simply co_await the promise */
	co_return co_await m_handle.promise();
}

template<typename T>
inline cTask<T> cAsyncGenerator<T>::Next() {
	promise_type& p = m_handle.promise();
	/* If a value is available, return it */
	if (co_await p) co_return p.publish();
	/* Otherwise throw */
	throw std::runtime_error("MEOW");
}
#endif //GREEKBOT_ASYNCGENERATOR_H