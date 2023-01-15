#ifndef GREEKBOT_TASK_H
#define GREEKBOT_TASK_H
#include <coroutine>
#include <exception>
#include <optional>
#include <atomic>
/* ========== A simple non-owning coroutine task ==================================================================== */
struct cDetachedTask {
	struct promise_type {
		cDetachedTask    get_return_object() const noexcept { return {}; }
		std::suspend_never initial_suspend() const noexcept { return {}; }
		std::suspend_never   final_suspend() const noexcept { return {}; }
		void return_void() const noexcept {}
		void unhandled_exception() const noexcept;
	};
};
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
		return *new(this) cTask(std::forward<cTask>(o));
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) {
		m_handle.promise().caller = h;
		return m_handle;
	}
	T await_resume();

	T Wait();

private:
	struct promise_base;
	std::coroutine_handle<promise_type> m_handle;

	cTask(std::coroutine_handle<promise_type> h) : std::suspend_always(), m_handle(h) {}
};

/* ================================================================================================================== */
template<typename T>
struct cTask<T>::promise_base : std::suspend_always {
	std::coroutine_handle<> caller; // The task coroutine caller
	std::exception_ptr      except; // Any unhandled exception that might occur

	cTask get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*static_cast<promise_type*>(this)); }
	std::suspend_always initial_suspend() noexcept { return {}; }
	promise_base   final_suspend() noexcept { return *this; }
	void unhandled_exception() noexcept { except = std::current_exception(); }

	std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept { return caller; }
};
/* ================================================================================================================== */
template<typename T>
struct cTask<T>::promise_type : promise_base {
	std::optional<T> value;
	template<typename U>
	void return_value(U&& v) { value.emplace(std::forward<U>(v)); }
	void return_value(T&& t) { value.emplace(std::forward<T>(t)); }
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
/* ================================================================================================================== */
template<typename T>
inline T cTask<T>::Wait() {
	std::optional<T>   value;
	std::exception_ptr except;
	std::atomic_bool   bDone = false;
	[&]() -> cDetachedTask {
		try {
			value.emplace(co_await *this);
		}
		catch (...) {
			except = std::current_exception();
		}
		bDone = true;
	} ();
	while (!bDone);
	if (except) std::rethrow_exception(except);
	return std::move(*value);
}
/* ================================================================================================================== */
template<>
void cTask<>::Wait();
#endif // GREEKBOT_TASK_H