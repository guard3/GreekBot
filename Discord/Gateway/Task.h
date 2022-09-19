#pragma once
#ifndef GREEKBOT_TASK_H
#define GREEKBOT_TASK_H
#include <coroutine>
#include <exception>
#include <concepts>
#include <memory>

namespace discord::detail {
	template<typename> struct promise;
	template<typename> struct promise_base;
}

template<typename T = void>
class cTask final {
public:
	typedef discord::detail::promise<T> promise_type;

	bool await_ready() { return m_handle.done(); }
	void await_suspend(std::coroutine_handle<> h);
	T    await_resume();

private:
	template<typename>
	friend class discord::detail::promise_base;
	std::coroutine_handle<promise_type> m_handle;

	cTask(std::coroutine_handle<promise_type> h) : m_handle(h) {}
};

namespace discord::detail {
/* ================================================================================================================== */
	template<typename R>
	struct promise_base {
		std::coroutine_handle<> caller; // The task coroutine caller
		std::exception_ptr      except; // Any unhandled exception that might occur

		cTask<R> get_return_object() { return std::coroutine_handle<promise<R>>::from_promise(static_cast<promise<R>&>(*this)); }
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
	struct promise : promise_base<R> {
		std::unique_ptr<R> value; // By default, an object might not be default-initializable, so we use a pointer
		void return_value(auto&& v) { value = std::make_unique<R>(std::forward<R>(v)); }
	};
/* ================================================================================================================== */
	template<typename R> requires std::default_initializable<R> && std::movable<R>
	struct promise<R> : promise_base<R> {
		R value;
		void return_value(auto&& v) { value = std::forward<R>(v); }
	};
/* ================================================================================================================== */
	template<>
	struct promise<void> : promise_base<void> {
		void return_void() {}
	};
}

template<typename T>
inline void cTask<T>::await_suspend(std::coroutine_handle<> h) { m_handle.promise().caller = h; }

template<typename T>
inline T cTask<T>::await_resume() {
	auto e = std::move(m_handle.promise().except);
	auto v = std::move(m_handle.promise().value);
	m_handle.destroy();
	if (e) std::rethrow_exception(e);
	if constexpr (std::default_initializable<T> && std::movable<T>)
		return std::move(v);
	else
		return std::move(*v);
}

template<>
inline void cTask<void>::await_resume() {
	auto e = std::move(m_handle.promise().except);
	m_handle.destroy();
	if (e) std::rethrow_exception(e);
}
#endif /* GREEKBOT_TASK_H */