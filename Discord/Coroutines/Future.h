#ifndef GREEKBOT_FUTURE_H
#define GREEKBOT_FUTURE_H
#include <coroutine>
#include <exception>
#include <future>
/* ========== Make std::future<> a valid coroutine return type ====================================================== */
template<typename T, typename... Args>
struct std::coroutine_traits<std::future<T>, Args...> {
	struct promise_type : std::promise<T> {
		std::future<T> get_return_object() { return this->get_future(); }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		template<typename U>
		void return_value(U&& u) { this->set_value(std::forward<U>(u)); }
		void return_value(T&& t) { this->set_value(std::forward<T>(t)); }
		void unhandled_exception() { this->set_exception(std::current_exception()); }
	};
};
/* ================================================================================================================== */
template<typename... Args>
struct std::coroutine_traits<std::future<void>, Args...> {
	struct promise_type : std::promise<void> {
		std::future<void> get_return_object() { return this->get_future(); }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() { this->set_value(); }
		void unhandled_exception() { this->set_exception(std::current_exception()); }
	};
};
#endif //GREEKBOT_FUTURE_H