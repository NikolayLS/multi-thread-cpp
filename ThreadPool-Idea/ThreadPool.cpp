#include "ThreadPool.h"

std::mutex ThreadPool::_singletonMutex;
ThreadPool* ThreadPool::_threadPoolInstance;

ThreadPool* ThreadPool::GetThreadPool()
{
	std::lock_guard<std::mutex> lock(_singletonMutex);
	if (_threadPoolInstance == nullptr)
	{
		static ThreadPool instance(16);
		_threadPoolInstance = &instance;
	}
	return _threadPoolInstance;
}

ThreadPool::ThreadPool(const unsigned short count)
{
	if (count > MAX_THREAD_COUNT) throw std::exception("Max_thread_count_exception_1");
	for (short i = 0;i < count;i++)
		AddThread();
}

ThreadPool::~ThreadPool()
{
	Ulock lock(_endMutex);
	_endCond.wait(	lock,
					[this]() {return _scheduledTaskCount == 0;}	); //lambda

	_mustDestroyPool = true;
	_cond.notify_all();

	for (short i = 0;i < (short)_threads.size() ;i++)
		_threads[i].join();
	lock.unlock();
}

bool ThreadPool::AddThread()
{
	if (_threads.size() + 1 > MAX_THREAD_COUNT) return false;
	_threads.push_back(std::thread(&ThreadPool::StartThread, this));
	return true;
}

void ThreadPool::StartThread()
{
	while (!_mustDestroyPool)
	{
		Ulock lock(_mutex);
		_cond.wait( lock,
					[this]() {return !_taskQueue.empty() || _mustDestroyPool;}); // lambda

		if (_mustDestroyPool) { lock.unlock(); break; }

		Task task = _taskQueue.front();
		_taskQueue.pop();
		lock.unlock();

		try {
			task(); //involk
			_scheduledTaskCount--;
			if (_scheduledTaskCount == 0) _endCond.notify_one();
		}
		catch (std::exception& e) {
			std::cerr << e.what();
			_scheduledTaskCount--;
			if (_scheduledTaskCount == 0) _endCond.notify_one();
		}
	}
}

void ThreadPool::ScheduleTask(const Task task)
{
	Ulock lock(_mutex);
	_scheduledTaskCount++;
	_taskQueue.push(task);
	lock.unlock();
	_cond.notify_one();
}