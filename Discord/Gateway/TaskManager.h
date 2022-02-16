#pragma once
#ifndef _GREEKBOT_TASKMANAGER_H_
#define _GREEKBOT_TASKMANAGER_H_
#include <thread>

class cTask final {
private:
	cTask*      m_next;   // The next node in the task list
	std::thread m_thread; // The task thread

public:
	template<typename Function, typename... Args>
	cTask(cTask* next, Function&& f, Args&&... args) : m_next(next), m_thread(std::forward<Function>(f), std::forward<Args>(args)...) {}
	cTask(const cTask&) = delete;
	cTask(cTask&&) = delete;
	~cTask() {
		delete m_next;
		if (m_thread.joinable())
			m_thread.join();
	}
};

class cTaskManager final {
	friend class cTask;
private:
	std::atomic<cTask*> m_head;
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
		cTask* p = m_head.exchange(nullptr);
		m_head.exchange(new cTask(p, std::forward<Function>(f), std::forward<Args>(args)...));
		return *this;
	}
};
#endif /* _GREEKBOT_TASKMANAGER_H_ */
