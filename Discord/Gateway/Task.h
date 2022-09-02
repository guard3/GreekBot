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
	struct promise_type_base {
		std::coroutine_handle<> caller; // The task coroutine caller
		std::exception_ptr      except; // Any unhandled exception that might occur

		std::suspend_never initial_suspend() const noexcept { return {}; }
		std::suspend_never   final_suspend() const noexcept { return {}; }
		void unhandled_exception() noexcept {
			/* Capture the unhandled exception to be rethrown on resume */
			except = std::current_exception();
			/* The caller at this point is suspended, so we resume it */
			if (caller) caller();
		}
	};

	std::coroutine_handle<promise_type> m_handle;

	cTask(auto&& h) : m_handle(std::forward<std::coroutine_handle<promise_type>>(h)) {}
};

template<typename T>
struct cTask<T>::promise_type : promise_type_base {
	std::unique_ptr<T> value;
	cTask<T> get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
	void return_value(auto&& v) {
		/* Save the return value */
		value = std::make_unique<T>(std::forward<T>(v));
		/* Resume the caller */
		if (this->caller) this->caller();
	}
};

template<>
struct cTask<void>::promise_type : promise_type_base {
	cTask<void> get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
	void return_void() {
		/* Resume the caller */
		if (this->caller) this->caller();
	}
};

template<typename T>
inline void cTask<T>::await_suspend(std::coroutine_handle<> h) { m_handle.promise().caller = h; }

template<typename T>
inline T cTask<T>::await_resume() {
	if (m_handle.promise().except)
		std::rethrow_exception(m_handle.promise().except);
	return std::move(*m_handle.promise().value);
}

template<>
inline void cTask<void>::await_resume() {
	if (m_handle.promise().except)
		std::rethrow_exception(m_handle.promise().except);
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
