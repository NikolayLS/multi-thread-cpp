#include <iostream>
#include <fstream>
#include "ThreadPool.h"

struct Square //Example class - just to show how this thread pool works
{
	unsigned int a = 1;
	std::mutex _mutex;
	void Area()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		std::ofstream file("Area.txt",std::ios::app);
		std::this_thread::sleep_for(std::chrono::seconds(10));
		file << a * a << '\n'; // this will be not thread safe if we do not use mutex
		//the files's destructor will close the file
		//lock_guard destructor will unlock the mutex
	}
};

int main()
{
	try {
		std::shared_ptr<Square>sq ( new Square());//need to be ptr - in [] are only consts 
		sq->a = 10;								  //but if we use ptr we can use member functions
		ThreadPool::GetThreadPool()->ScheduleTask([sq]() {sq->Area();}); //lambda function
		std::cout << "while doing the asynchronized task - print this\n";
	}
	catch (std::exception& e) {
		std::cerr << e.what();
	}
	return 0;
}