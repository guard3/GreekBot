#include "Task.h"
#include "Common.h"

void
cDetachedTask::promise_type::unhandled_exception() const noexcept {
	try {
		cUtils::PrintErr("An exception occurred on a detached task");
		std::rethrow_exception(std::current_exception());
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("With message: %s", e.what());
	}
	catch (...) {}
}

template<>
void cTask<>::Wait() {
	std::exception_ptr except;
	std::atomic_bool   bDone = false;
	[&]() -> cDetachedTask {
		try {
			co_await *this;
		}
		catch (...) {
			except = std::current_exception();
		}
		bDone = true;
	} ();
	while (!bDone);
	if (except) std::rethrow_exception(except);
}