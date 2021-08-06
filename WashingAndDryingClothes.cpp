#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

namespace Test {
	void Assert(bool flag, const char* err)
	{
		if (!flag) throw std::exception(err);
		else (std::cout << "TEST PASSED\n");
	}
}

enum ClothesState { Dirty, CleanAndWet, CleanAndDry };

void dry(ClothesState& c)
{
	Test::Assert(c == ClothesState::CleanAndWet, "Err line 19");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	c = ClothesState::CleanAndDry;
}

void wash(ClothesState& c)
{
	Test::Assert(c == ClothesState::Dirty, "Err line 26");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	c = ClothesState::CleanAndWet;
}

void WashingAndDryingClothes(std::vector<ClothesState>& arr)
{ // WashingAndDryingClothes works with 1 washing machine and 1 drying machine so the work must be done in arr.size() + 1 * time for washing and drying
	// Assume that the time for washing es equal to the time for drying
	for (size_t i = 0;i < arr.size();i++)
	{
		std::thread washingThread(wash, std::ref(arr[i]));
		if (i > 0)
		{
			std::thread dryingThread(dry, std::ref(arr[i - 1]));
			dryingThread.join();
		}
		washingThread.join();
	}
	dry(arr[arr.size() -1]);
}

void CleanClothesCheck(std::vector<ClothesState>& arr)
{
	for (size_t i = 0;i < arr.size();i++)
		Test::Assert(arr[i] == ClothesState::CleanAndDry, "Err line 50");
}

int main()
{
	try
	{
		std::vector<ClothesState> arr;
		for (size_t i = 0;i < 10000;i++)
			arr.push_back(ClothesState(ClothesState::Dirty));

		WashingAndDryingClothes(arr);
		CleanClothesCheck(arr);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
	}

	return 0;
}
