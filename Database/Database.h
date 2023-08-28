#ifndef GREEKBOT_DATABASE_H
#define GREEKBOT_DATABASE_H
#include <stdexcept>
#include <coroutine>
#include <vector>
#include <functional>
#include "Message.h"
/* ================================================================================================================== */
class xDatabaseError : public std::runtime_error {
private:
	int m_code;
public:
	xDatabaseError();
	int code() const noexcept { return m_code; }
};
/* ================================================================================================================== */
class cRankQueryDataElement final {
private:
	int64_t rank;
	cSnowflake id;
	int64_t xp;
	int64_t num_msg;

public:
	cRankQueryDataElement(int64_t rank, cSnowflake id, int64_t xp, int64_t num_msg) : rank(rank), id(id), xp(xp), num_msg(num_msg) {};

	int64_t GetRank()        const { return rank;    }
	int64_t GetXp()          const { return xp;      }
	int64_t GetNumMessages() const { return num_msg; }

	chSnowflake GetUserId() const { return &id; }
};
typedef std::vector<cRankQueryDataElement> tRankQueryData;
/* ================================================================================================================== */
template<typename T = void>
class cDatabaseTask;
/* ================================================================================================================== */
class cDatabase final {
private:
	static cDatabase ms_instance;
	cDatabase();
public:
	cDatabase(const cDatabase&) = delete;
	~cDatabase();

	cDatabase& operator=(const cDatabase&) = delete;

	static cDatabaseTask<> UpdateLeaderboard(const cMessage&);
	static cDatabaseTask<tRankQueryData> GetUserRank(const cUser&);
	static cDatabaseTask<tRankQueryData> GetTop10();

	static cDatabaseTask<uint64_t> WC_RegisterMember(const cMember&);
	static cDatabaseTask<> WC_UpdateMessage(const cUser&, const cMessage&);
	static cDatabaseTask<int64_t> WC_GetMessage(const cPartialMember&);
	static cDatabaseTask<> WC_EditMessage(int64_t);
	static cDatabaseTask<uint64_t> WC_DeleteMember(const cUser&);
};
/* ================================================================================================================== */
template<>
class cDatabaseTask<void> {
protected:
	std::function<void(std::coroutine_handle<>)> m_func;
	std::exception_ptr m_except;

	template<typename F> requires std::is_invocable_r_v<void, F, std::coroutine_handle<>>
	cDatabaseTask(F&& f) : m_func(std::forward<F>(f)) {}

public:
	template<typename F> requires std::is_invocable_r_v<void, F>
	cDatabaseTask(F&& f) : m_func([this, f = std::forward<F>(f)](std::coroutine_handle<> h) {
		try {
			f();
		}
		catch (...) {
			m_except = std::current_exception();
		}
		h();
	}) {}

	constexpr bool await_ready() noexcept { return false; }
	void await_suspend(std::coroutine_handle<> h);
	void await_resume() { if (m_except) std::rethrow_exception(m_except); }
};
/* ================================================================================================================== */
template<std::default_initializable T>
class cDatabaseTask<T> : public cDatabaseTask<void> {
private:
	T m_result;

public:
	template<typename F> requires std::is_invocable_r_v<void, F>
	cDatabaseTask(F&& f) : cDatabaseTask<void>([this, f = std::forward<F>(f)](std::coroutine_handle<> h) {
		try {
			m_result = f();
		}
		catch (...) {
			m_except = std::current_exception();
		}
		h();
	}) {}

	T await_resume() {
		cDatabaseTask<void>::await_resume();
		return std::move(m_result);
	}
};
/* ================================================================================================================== */
template<typename T>
class cDatabaseTask : public cDatabaseTask<void> {
private:
	uhHandle<T> m_result;

public:
	template<typename F> requires std::is_invocable_r_v<void, F>
	cDatabaseTask(F&& f) : cDatabaseTask<void>([this, f = std::forward<F>(f)](std::coroutine_handle<> h) {
		try {
			m_result = cHandle::MakeUnique<T>(f());
		}
		catch (...) {
			m_except = std::current_exception();
		}
		h();
	}) {}

	T await_resume() {
		cDatabaseTask<void>::await_resume();
		return std::move(*m_result);
	}
};
#endif /* GREEKBOT_DATABASE_H */