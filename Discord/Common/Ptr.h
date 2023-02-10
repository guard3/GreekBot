#ifndef GREEKBOT_PTR_H
#define GREEKBOT_PTR_H
#include <compare>
#include <stdexcept>

class xNullPtrError : public std::runtime_error {
public:
	xNullPtrError() : std::runtime_error("Null pointer dereference") {}
};

template<typename T>
class cPtr {
private:
	T* m_ptr;

public:
	cPtr() noexcept : m_ptr(nullptr) {}
	cPtr(T* p) noexcept : m_ptr(p) {}
	template<typename U> requires std::is_constructible_v<T*, U*>
	cPtr(U* p) noexcept : m_ptr(p) {}
	template<typename U> requires std::is_constructible_v<T*, U*>
	cPtr(const cPtr<U>& p) noexcept : m_ptr(p.Get()) {}

	T* Get() const noexcept { return m_ptr; }

	auto operator<=>(const cPtr&) const = default;

	T* operator->() const {
		if (!m_ptr) throw xNullPtrError();
		return m_ptr;
	}
	T& operator*() const {
		if (!m_ptr) throw xNullPtrError();
		return *m_ptr;
	}

	operator bool() const noexcept { return m_ptr; }
};

template<typename T>
class cPtr<T[]> : public cPtr<T> {
public:
	cPtr() noexcept : cPtr<T>() {}
	cPtr(T* p) noexcept : cPtr<T>(p) {}

	T& operator[](size_t i) const {
		if (!this->Get()) throw xNullPtrError();
		return this->Get()[i];
	}
};

template<>
class cPtr<void> {
private:
	void* m_ptr;

public:
	cPtr() noexcept : m_ptr(nullptr) {}
	cPtr(void* p) noexcept : m_ptr(p) {}

	void* Get() const noexcept { return m_ptr; }

	auto operator<=>(const cPtr&) const = default;
};
#endif //GREEKBOT_PTR_H
