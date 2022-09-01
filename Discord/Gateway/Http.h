#ifndef _GREEKBOT_HTTP_H_
#define _GREEKBOT_HTTP_H_
#include <string>
#include <thread>
#include <tuple>
#include <coroutine>
#include "Discord.h"
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
namespace beast = boost::beast;
namespace asio  = boost::asio;

#include "Common.h"
// TODO: move those to common.h
template<typename T = void>
class cTask2 final {
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

	cTask2(auto&& h) : m_handle(std::forward<std::coroutine_handle<promise_type>>(h)) {}
};

template<typename T>
struct cTask2<T>::promise_type : promise_type_base {
	T value;
	cTask2<T> get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
	void return_value(auto&& v) {
		/* Save the return value */
		value = std::forward<T>(v);
		/* Resume the caller */
		if (this->caller) this->caller();
	}
};

template<>
struct cTask2<void>::promise_type : promise_type_base {
	cTask2<void> get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
	void return_void() {
		/* Resume the caller */
		if (this->caller) this->caller();
	}
};

template<typename T>
inline void cTask2<T>::await_suspend(std::coroutine_handle<> h) { m_handle.promise().caller = h; }

template<typename T>
inline T cTask2<T>::await_resume() {
	if (m_handle.promise().except)
		std::rethrow_exception(m_handle.promise().except);
	return std::move(m_handle.promise().value);
}

template<>
inline void cTask2<void>::await_resume() {
	if (m_handle.promise().except)
		std::rethrow_exception(m_handle.promise().except);
}

#if 0
template<typename T>
class cPromise;

template<typename T = void>
struct cTask_ : std::coroutine_handle<cPromise<T>> {
	typedef cPromise<T> promise_type;

	bool await_ready();
	void await_suspend(std::coroutine_handle<>);
	T await_resume();

};

template<typename T>
class cBasePromise {
public:
	std::coroutine_handle<> caller;
	std::exception_ptr except;

	std::suspend_never initial_suspend() const noexcept { return {}; }
	std::suspend_never final_suspend() const noexcept { return {}; }
	void unhandled_exception() noexcept {
		cUtils::PrintErr("An exception occurred.");
		except = std::current_exception();
		if (caller)
			caller();
	}
};

template<typename T>
class cPromise : public cBasePromise<T> {
public:
	T value;

	cTask_<T> get_return_object() noexcept { return { cTask_<T>::from_promise(*this) }; }
	void return_value(T v) {
		this->value = v;
		if (this->caller)
			this->caller();
	}
};

template<>
class cPromise<void> : public cBasePromise<void> {
public:
	cTask_<> get_return_object() noexcept { return { cTask_<>::from_promise(*this) }; }
	void return_void() {
		if (this->caller)
			this->caller();
	}
};

template<typename T>
inline bool cTask_<T>::await_ready() {
	cUtils::PrintLog("Checking if we're ready...");
	return this->done();
}
template<typename T>
inline void cTask_<T>::await_suspend(std::coroutine_handle<> h) {
	cUtils::PrintLog("Suspending...");
	this->promise().caller = h;
}
template<typename T>
inline T cTask_<T>::await_resume() {
	cUtils::PrintLog("Resuming...");
	if (this->promise().except) {
		cUtils::PrintErr("Yes, it indeed occurred.");
		std::rethrow_exception(this->promise().except);
	}
	return this->promise().value;
}
template<>
inline void cTask_<void>::await_resume() {
	cUtils::PrintLog("Resuming...");
	if (this->promise().except) {
		cUtils::PrintErr("Yes, it indeed occurred.");
		std::rethrow_exception(this->promise().except);
	}
}
#endif
class cHttpField final {
private:
	const char *m_name, *m_value;
public:
	cHttpField(const char* n, const char* v) : m_name(n), m_value(v) {}
	const char* GetName()  const noexcept { return m_name;  }
	const char* GetValue() const noexcept { return m_value; }
};

typedef cTask2<std::tuple<unsigned int, std::string>> tHttpTask;
typedef std::initializer_list<cHttpField> tHttpFields;

class cHttp {
private:
	/* The io and ssl contexts */
	asio::io_context   m_ioc;
	asio::ssl::context m_ctx;
	/* A work guard to prevent m_ioc.run() from running out of work */
	asio::executor_work_guard<asio::io_context::executor_type> m_work;
	/* A buffer to receive http responses */
	beast::flat_buffer m_buffer;
	/* A thread to execute async io handlers */
	std::thread m_run_thread;

public:
	cHttp() : m_ctx(asio::ssl::context::tlsv13_client), m_work(asio::make_work_guard(m_ioc)), m_run_thread([this]() { m_ioc.run(); }) {
		/* Set SSL context to verify peer */
		m_ctx.set_default_verify_paths();
		m_ctx.set_verify_mode(asio::ssl::verify_peer);
	}
	~cHttp() {
		/* Complete any remaining tasks and let io_context::run exit */
		m_work.reset();
		/* Join the thread */
		m_run_thread.join();
	}

	cTask2<> Test() {
		unsigned int s;
		std::string str;
		std::tie(s, str) = co_await Get("discord.com", "/api/gateway");
		cUtils::PrintLog("%d\n%s", s, str);
	}

	tHttpTask    Get(std::string host, std::string target,                      tHttpFields fields = {});
	tHttpTask   Post(std::string host, std::string target, std::string content, tHttpFields fields = {});
	tHttpTask  Patch(std::string host, std::string target, std::string content, tHttpFields fields = {});
	tHttpTask    Put(std::string host, std::string target, std::string content, tHttpFields fields = {});
	tHttpTask Delete(std::string host, std::string target, std::string content, tHttpFields fields = {});
};
#endif /* _GREEKBOT_HTTP_H_ */