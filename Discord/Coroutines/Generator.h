#ifndef GREEKBOT_GENERATOR_H
#define GREEKBOT_GENERATOR_H
#include "Task.h"
#include <stdexcept>
// TODO: make a generator specific exception
template<typename T>
class cGenerator {
public:
	struct promise_type;

	cGenerator(const cGenerator&) = delete;
	cGenerator(cGenerator&& o) noexcept : m_handle(o.m_handle) { o.m_handle = nullptr; }
	~cGenerator() { if (m_handle) m_handle.destroy(); }

	cGenerator& operator=(cGenerator o) noexcept {
		swap(m_handle, o.m_handle);
		return *this;
	}

	bool HasValue();
	T Next();

	operator bool() { return HasValue(); }
	T operator()() { return Next(); }

private:
	std::coroutine_handle<promise_type> m_handle;

	cGenerator(std::coroutine_handle<promise_type> h) : m_handle(h) {}
};

template<typename T>
struct cGenerator<T>::promise_type {
	std::exception_ptr except;
	std::optional<T> value;
	bool ready = false;

	cGenerator get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
	std::suspend_always initial_suspend() { return {}; }
	std::suspend_always final_suspend() noexcept {
		ready = true;
		return {};
	}
	template<typename U = T>
	std::suspend_always yield_value(U&& u) {
		ready = true;
		value = std::forward<U>(u);
		return {};
	}
	void return_void() {}
	void unhandled_exception() { except = std::current_exception(); }
};

template<typename T>
inline bool cGenerator<T>::HasValue() {
	promise_type& p = m_handle.promise();
	if (!p.ready) m_handle.resume();
	if (p.except) std::rethrow_exception(p.except);
	return !m_handle.done();
}

template<typename T>
inline T cGenerator<T>::Next() {
	if (HasValue()) {
		promise_type& p = m_handle.promise();
		if (p.ready) {
			p.ready = false;
			return std::move(*p.value);
		}
	}
	throw std::runtime_error("Generator has no value");
}
#endif //GREEKBOT_GENERATOR_H