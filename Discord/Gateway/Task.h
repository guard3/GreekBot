#pragma once
#ifndef GREEKBOT_TASK_H
#define GREEKBOT_TASK_H
#include <coroutine>
#include <exception>

template<typename T = void>
class cTask final {
public:
	struct promise_type;

	bool await_ready() { return m_handle.done(); }
	void await_suspend(std::coroutine_handle<> h);
	T    await_resume();

private:
	typedef std::coroutine_handle<promise_type> tCoro;
	struct promise_type_base {
		std::coroutine_handle<> caller; // The task coroutine caller
		std::exception_ptr      except; // Any unhandled exception that might occur

		cTask get_return_object() { return tCoro::from_promise(static_cast<promise_type&>(*this)); }
		auto initial_suspend() noexcept { return std::suspend_never{}; }
		auto   final_suspend() noexcept {
			struct awaitable {
				std::coroutine_handle<>& caller;
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

	tCoro m_handle;

	cTask(tCoro h) : m_handle(h) {}
};

template<typename T>
struct cTask<T>::promise_type : promise_type_base {
	std::unique_ptr<T> value;
	void return_value(auto&& v) { value = std::make_unique<T>(std::forward<T>(v)); }
};

template<>
struct cTask<void>::promise_type : promise_type_base {
	void return_void() {}
};

template<typename T>
inline void cTask<T>::await_suspend(std::coroutine_handle<> h) { m_handle.promise().caller = h; }

template<typename T>
inline T cTask<T>::await_resume() {
	auto e = std::move(m_handle.promise().except);
	auto p = std::move(m_handle.promise().value);
	m_handle.destroy();
	if (e) std::rethrow_exception(e);
	return std::move(*p);
}

template<>
inline void cTask<void>::await_resume() {
	auto e = std::move(m_handle.promise().except);
	m_handle.destroy();
	if (e) std::rethrow_exception(e);
}
#endif /* GREEKBOT_TASK_H */