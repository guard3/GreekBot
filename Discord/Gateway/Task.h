#ifndef GREEKBOT_TASK_H
#define GREEKBOT_TASK_H
#include <coroutine>
#include <exception>
#include <optional>

template<typename T = void>
class cTask final {
public:
	struct promise_type;

	bool await_ready() { return m_handle.done(); }
	void await_suspend(std::coroutine_handle<> h);
	T    await_resume();

private:
	struct promise_base;
	std::coroutine_handle<promise_type> m_handle;

	cTask(std::coroutine_handle<promise_type> h) : m_handle(h) {}
};

/* ================================================================================================================== */
template<typename R>
struct cTask<R>::promise_base {
	std::coroutine_handle<> caller; // The task coroutine caller
	std::exception_ptr      except; // Any unhandled exception that might occur

	cTask<R> get_return_object() { return std::coroutine_handle<promise_type>::from_promise(static_cast<promise_type&>(*this)); }
	auto initial_suspend() noexcept { return std::suspend_never{}; }
	auto   final_suspend() noexcept {
		struct awaitable {
			std::coroutine_handle<> caller;
			bool await_ready() noexcept { return false; }
			bool await_suspend(std::coroutine_handle<> h) noexcept {
				if (caller) caller();
				return true;
			}
			void await_resume() noexcept {}
		};
		return awaitable{caller};
	}
	void unhandled_exception() noexcept { except = std::current_exception(); }
};
/* ================================================================================================================== */
template<typename R>
struct cTask<R>::promise_type : promise_base {
	std::optional<R> value;
	void return_value(auto&& v) { value.emplace(std::forward<R>(v)); }
};
/* ================================================================================================================== */
template<>
struct cTask<void>::promise_type : promise_base {
	void return_void() {}
};

template<typename T>
inline void cTask<T>::await_suspend(std::coroutine_handle<> h) { m_handle.promise().caller = h; }

template<typename T>
inline T cTask<T>::await_resume() {
	auto e = std::move(m_handle.promise().except);
	auto v = std::move(m_handle.promise().value);
	m_handle.destroy();
	if (e) std::rethrow_exception(e);
	return std::move(*v);
}

template<>
inline void cTask<void>::await_resume() {
	auto e = std::move(m_handle.promise().except);
	m_handle.destroy();
	if (e) std::rethrow_exception(e);
}
#endif // GREEKBOT_TASK_H