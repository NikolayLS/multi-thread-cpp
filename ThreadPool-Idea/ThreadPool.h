#pragma once
#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

//typedefs
typedef std::function<void()> Task;
typedef std::queue<Task> WorkQueue;
typedef std::vector<std::thread> ThreadArr;
typedef std::unique_lock<std::mutex> Ulock;
//typedefs

class ThreadPool // Thread safe singleton class
{
private:

	static constexpr unsigned short MAX_THREAD_COUNT = 64;

	//basic thread pool-data {
	WorkQueue _taskQueue;
	ThreadArr _threads;
	std::mutex _mutex;
	std::condition_variable _cond; //}

	//thread safe singleton class-data {
	static ThreadPool* _threadPoolInstance;
	static std::mutex _singletonMutex; //}

	//destructor behaviour-data {
	mutable bool _mustDestroyPool = false;
	std::atomic<int> _scheduledTaskCount = 0;
	std::mutex _endMutex;
	std::condition_variable _endCond; //}

	//Private constructor and deleted cpy constructor and operator= {
	ThreadPool(const unsigned short count);
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete; // }

	void StartThread();

public:

	static ThreadPool* GetThreadPool();// Get the only thread pool instance
	void ScheduleTask(Task task);
	bool AddThread();
	~ThreadPool();
};

