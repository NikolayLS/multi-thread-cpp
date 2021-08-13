#include <iostream>
#include <queue>
#include <mutex>
#include <vector>

//Assert
namespace Test {
	void Assert(bool predicate, const char* error_message)
	{
		if (!predicate) throw std::exception(error_message);
	}

	template<typename T>
	void AssertEqual(T first,T second, const char* error_message)
	{
		if ( !(first== second )) throw std::exception(error_message);
	}
}

// typedefs
typedef unsigned int Uint;
typedef std::unique_lock<std::mutex> Ulock;
// typedefs

template <typename T>
class ThreadSafeQueue // Producer and consumer task - we must use only pointer so the obects in queue represents real objects 
{					  // For the task - we need to include mutex and condition variable
private:

	std::queue<T> _workQueue;
	Uint _maxSize;

	std::mutex _mutex;
	std::condition_variable _cond;

	ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;
	ThreadSafeQueue(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue(ThreadSafeQueue&&) = delete;

	bool isEmpty() { return _workQueue.size() == 0; }
	bool isFull() { return (_workQueue.size() >= _maxSize); }

public:
	ThreadSafeQueue(Uint maxSize) : _maxSize(maxSize)
	{
		Test::Assert(std::is_pointer<T>::value, "This queue works only with pointer types");
	}

	void Produce(const T produce)
	{
		Ulock lock(_mutex);
		_cond.wait( lock,
					[this]() {return !isFull();}	); //lambda

		_workQueue.push(produce);
		std::cout << *produce << " has been produced\n";

		lock.unlock();
		_cond.notify_one();
	}

	T Consume() //T is always pointer type
	{
		Ulock lock(_mutex);
		_cond.wait( lock,
					[this]() {return !isEmpty();}	); //lambda

		T forReturn = _workQueue.front();
		_workQueue.pop();
		std::cout << *forReturn << " has been consumed\n";

		lock.unlock();
		_cond.notify_one();

		return forReturn;
	}
};

// Help function for testing ...

void producing(std::vector<int*>& arr, ThreadSafeQueue<int*>& q)
{

	for (size_t i = 0;i < arr.size();i++)
	{
		q.Produce(arr[i]);
		std::this_thread::sleep_for(std::chrono::microseconds(10)); // for randomizing producing and consuming order
	}
}

void consuming(ThreadSafeQueue<int*>& q)
{
	while (1)
	{
		q.Consume();
		std::this_thread::sleep_for(std::chrono::microseconds(10)); // for randomizing producing and consuming order
	}
}

int main()
{
	try
	{
		int integers[20] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };

		ThreadSafeQueue<int*> pac(10); // 10 is max-size

		std::vector<int*> arr;
		for (size_t i = 0;i < 20;i++)
			arr.push_back(&integers[i]);

		std::thread t1(producing, std::ref(arr), std::ref(pac));
		std::thread t2(consuming, std::ref(pac));

		t1.join();
		t2.join();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
	}
	return 0;
}
