#ifndef GREEKBOT_TASK_H
#define GREEKBOT_TASK_H
#include <coroutine>
#include <exception>
#include <future>
#include <optional>
/* ========== A 'lazy' coroutine task with symmetric transfer ======================================================= */
template<typename T = void>
class cTask final : public std::suspend_always {
public:
	struct promise_type;

	cTask(const cTask&) = delete;
	cTask(cTask&& o) noexcept : cTask(o.m_handle) { o.m_handle = nullptr; }
	~cTask() { if (m_handle) m_handle.destroy(); }

	cTask& operator=(const cTask&) = delete;
	cTask& operator=(cTask&& o) noexcept {
		~cTask();
		return *new(this) cTask(std::move(o));
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) {
		m_handle.promise().caller = h;
		return m_handle;
	}
	T await_resume();

	T Wait() { return wait(this).get(); }

private:
	struct promise_base;
	std::coroutine_handle<promise_type> m_handle;

	cTask(std::coroutine_handle<promise_type> h) : std::suspend_always(), m_handle(h) {}

	static std::future<T> wait(cTask* task) { co_return co_await *task; }
};
/* ================================================================================================================== */
template<typename T>
struct cTask<T>::promise_base : std::suspend_always {
	std::coroutine_handle<> caller; // The task coroutine caller
	std::exception_ptr      except; // Any unhandled exception that might occur

	cTask get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*static_cast<promise_type*>(this)); }
	std::suspend_always initial_suspend() noexcept { return {}; }
	promise_base& final_suspend() noexcept { return *this; }
	void unhandled_exception() noexcept { except = std::current_exception(); }

	std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept { return caller; }
};
/* ================================================================================================================== */
template<typename T>
struct cTask<T>::promise_type : promise_base {
	std::optional<T> value;
	template<typename U = T>
	void return_value(U&& v) { value.emplace(std::move(v)); }
};
/* ================================================================================================================== */
template<>
struct cTask<void>::promise_type : promise_base {
	void return_void() {}
};
/* ================================================================================================================== */
template<typename T>
inline T cTask<T>::await_resume() {
	promise_type& p = m_handle.promise();
	if (p.except) std::rethrow_exception(p.except);
	return std::move(*p.value);
}
/* ================================================================================================================== */
template<>
inline void cTask<>::await_resume() {
	promise_type& p = m_handle.promise();
	if (p.except) std::rethrow_exception(p.except);
}
/* ========== Make std::future a coroutine type; used for blocking and waiting on tasks ============================= */
template<typename T>
struct std::coroutine_traits<std::future<T>, cTask<T>*> {
	struct promise_type : std::promise<T> {
		std::future<T> get_return_object() { return this->get_future(); }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		template<typename U = T>
		void return_value(U&& u) { this->set_value(std::move(u)); }
		void unhandled_exception() { this->set_exception(std::current_exception()); }
	};
};
/* ================================================================================================================== */
template<>
struct std::coroutine_traits<std::future<void>, cTask<void>*> {
	struct promise_type : std::promise<void> {
		std::future<void> get_return_object() { return this->get_future(); }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() { this->set_value(); };
		void unhandled_exception() { this->set_exception(std::current_exception()); }
	};
};
#endif // GREEKBOT_TASK_H