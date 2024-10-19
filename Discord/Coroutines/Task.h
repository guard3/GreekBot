#ifndef GREEKBOT_TASK_H
#define GREEKBOT_TASK_H
#include <coroutine>
#include <exception>
#include <optional>
#include <utility>
/* ========== A 'lazy' coroutine task with symmetric transfer ======================================================= */
template<typename T = void>
class cTask {
public:
	struct promise_type;

	cTask(const cTask&) = delete;
	cTask(cTask&& o) noexcept : cTask(std::exchange(o.m_handle, {})) {}
	~cTask() { if (m_handle) m_handle.destroy(); }

	cTask& operator=(cTask o) noexcept {
		swap(m_handle, o.m_handle);
		return *this;
	}

	constexpr bool await_ready() const noexcept { return false; }
	std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) const {
		m_handle.promise().caller = h;
		return m_handle;
	}
	T await_resume() const;

private:
	struct promise_base;
	std::coroutine_handle<promise_type> m_handle;

	cTask(std::coroutine_handle<promise_type> h) noexcept : m_handle(h) {}
};
/* ================================================================================================================== */
template<typename T>
struct cTask<T>::promise_base {
	std::coroutine_handle<> caller; // The task coroutine caller
	std::exception_ptr      except; // Any unhandled exception that might occur

	constexpr bool await_ready() const noexcept { return false; }
	constexpr void await_resume() const noexcept {}
	std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept { return caller; }

	cTask get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*static_cast<promise_type*>(this)); }
	std::suspend_always initial_suspend() noexcept { return {}; }
	const promise_base& final_suspend() noexcept { return *this; }
	void unhandled_exception() noexcept { except = std::current_exception(); }
};
/* ================================================================================================================== */
template<typename T>
struct cTask<T>::promise_type : promise_base {
	std::optional<T> value;
	template<typename U = T>
	void return_value(U&& v) noexcept(std::is_nothrow_constructible_v<T, U&&>) { value.emplace(std::move(v)); }
};
/* ================================================================================================================== */
template<>
struct cTask<void>::promise_type : cTask<void>::promise_base {
	constexpr void return_void() noexcept {}
};
/* ================================================================================================================== */
template<typename T>
inline T cTask<T>::await_resume() const {
	promise_type& p = m_handle.promise();
	if (p.except) std::rethrow_exception(p.except);
	return std::move(*p.value);
}
/* ================================================================================================================== */
template<>
inline void cTask<>::await_resume() const {
	promise_type& p = m_handle.promise();
	if (p.except) std::rethrow_exception(p.except);
}
#endif // GREEKBOT_TASK_H