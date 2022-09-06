#pragma once
#ifndef _GREEKBOT_TASKMANAGER_H_
#define _GREEKBOT_TASKMANAGER_H_
#include <thread>
#include <atomic>
#include <coroutine>

template<typename T = void>
class cTask final {
public:
	struct promise_type;

	bool await_ready() { return m_handle.done(); }
	void await_suspend(std::coroutine_handle<> h);
	T    await_resume();

private:
	typedef std::coroutine_handle<promise_type> coro_t;
	struct promise_type_base {
		std::coroutine_handle<> caller; // The task coroutine caller
		std::exception_ptr      except; // Any unhandled exception that might occur

		cTask get_return_object() { return coro_t::from_promise(static_cast<promise_type&>(*this)); }
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

	coro_t m_handle;

	cTask(auto&& h) : m_handle(std::forward<coro_t>(h)) {}
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
	return *p;
}

template<>
inline void cTask<void>::await_resume() {
	auto e = std::move(m_handle.promise().except);
	m_handle.destroy();
	if (e) std::rethrow_exception(e);
}

/* TODO: Delete this when done */
class cTasky final {
private:
	cTasky*      m_next;   // The next node in the task list
	std::thread m_thread; // The task thread

public:
	template<typename Function, typename... Args>
	cTasky(cTasky* next, Function&& f, Args&&... args) : m_next(next), m_thread(std::forward<Function>(f), std::forward<Args>(args)...) {}
	cTasky(const cTasky&) = delete;
	cTasky(cTasky&&) = delete;
	~cTasky() {
		delete m_next;
		if (m_thread.joinable())
			m_thread.join();
	}
};

class cTaskManager final {
	friend class cTasky;
private:
	std::atomic<cTasky*> m_head;
	std::atomic<bool>   m_bRun;
	std::thread         m_thread;

public:
	cTaskManager() : m_head(nullptr), m_bRun(true) { m_thread = std::thread([this]() { while (m_bRun.load()) delete m_head.exchange(nullptr); }); }
	cTaskManager(const cTaskManager&) = delete;
	cTaskManager(cTaskManager&&) = delete;
	~cTaskManager() {
		m_bRun.store(false);
		m_thread.join();
		delete m_head.load();
	}

	cTaskManager& operator=(cTaskManager) = delete;

	template<typename Function, typename... Args>
	cTaskManager& CreateTask(Function&& f, Args&&... args) {
		cTasky* p = m_head.exchange(nullptr);
		m_head.exchange(new cTasky(p, std::forward<Function>(f), std::forward<Args>(args)...));
		return *this;
	}
};
#endif /* _GREEKBOT_TASKMANAGER_H_ */
