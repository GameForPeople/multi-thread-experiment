#include "stdafx.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

#include <Array>
#include <vector>

namespace GLOBAL
{
	constexpr static int LOOP_COUNT = 50000000;
	constexpr static int THREAD_NUMBER_TYPE = 5;

	constexpr static int ADD_VALUE = 2;
	volatile int globalSum = 0;
	std::mutex globalLock;

	class Bakery {
	public:
		std::vector<bool> enterCont;
		std::vector<int> ticketCont;
		int threadNum;

		Bakery(int threadNum)
			: enterCont()
			, ticketCont()
			, threadNum(threadNum)
		{
			enterCont.reserve(threadNum);
			ticketCont.reserve(threadNum);

			for (int i = 0; i < threadNum; ++i)
			{
				enterCont.emplace_back(false);
				ticketCont.emplace_back(0);
			}
		}

		~Bakery()
		{
			enterCont.clear();
			ticketCont.clear();
		}

		void lock(int threadId) 
		{
			enterCont[threadId] = true;
			{
				int max = ticketCont[0];
				for (int i = 1; i < threadNum; ++i)
				{
					if (ticketCont[i] > max)
					{
						max = ticketCont[i];
					}
				}
				ticketCont[threadId] = max + 1;
			}
			enterCont[threadId] = false;

			for (int i = 0; i < threadNum; ++i) 
			{
				if (threadId == i) 
				{
					continue;
				}
				else 
				{
					while (enterCont[i]);
					while ((ticketCont[i] != 0) && ((ticketCont[i], i) < (ticketCont[threadId], threadId)));
				}
			}
		}

		void unlock(int threadId)
		{
			ticketCont[threadId] = 0;
		}
	};

	std::unique_ptr<Bakery> bakeryLock;
} using namespace GLOBAL;

namespace NO_LOCK
{
	void DoSum(int threadNum)
	{
		for (int i = 0, localLoopCount = LOOP_COUNT / threadNum; i < localLoopCount; ++i)
		{
			globalSum += 2;
		}
	}
}

namespace MUTEX
{
	void DoSum(int threadNum)
	{
		for (int i = 0, localLoopCount = LOOP_COUNT / threadNum; i < localLoopCount; ++i)
		{
			std::unique_lock<std::mutex> tempLock(globalLock);
			globalSum += 2;
		}
	}
}

namespace BAKERY
{
	void DoSum(int threadNum, int threadID)
	{
		for (int i = 0, localLoopCount = LOOP_COUNT / threadNum; i < localLoopCount; ++i)
		{
			bakeryLock->lock(threadID);		// ++++++++++++++++++1
			globalSum += 2;
			bakeryLock->unlock(threadID);	// ------------------0
		}
	}
}

int main()
{
	using namespace std::chrono;

	std::array<int, THREAD_NUMBER_TYPE> threadNumArr;
	threadNumArr[0] = 1;
	threadNumArr[1] = 2;
	threadNumArr[2] = 4;
	threadNumArr[3] = 8;
	threadNumArr[4] = 16;
	
	for (int i = 0; i < THREAD_NUMBER_TYPE; ++i)
	{
		std::vector<std::thread> threadCont;
		threadCont.reserve(threadNumArr[i]);

		globalSum = 0;

		auto startTime = high_resolution_clock::now();
		for (int j = 0; j < threadNumArr[i]; ++j) { threadCont.emplace_back(NO_LOCK::DoSum, threadNumArr[i]); }
		for (auto& thread : threadCont) { thread.join(); }
		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "No Lock : Thread Count "<< threadNumArr[i] <<" 합은? " << globalSum <<"성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
	}

	std::system("PAUSE");
}