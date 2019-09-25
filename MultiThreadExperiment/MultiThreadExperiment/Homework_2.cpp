#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

#include <Array>
#include <vector>

class Bakery {
public:
	volatile bool* enterCont;
	volatile int* ticketCont;

	int threadNum;

	Bakery(int threadNum)
		: enterCont()
		, ticketCont()
		, threadNum(threadNum)
	{
		enterCont = new volatile bool[threadNum];
		ticketCont = new volatile int[threadNum];

		for (int i = 0; i < threadNum; ++i)
		{
			enterCont[i] = false;
			ticketCont[i] = 0;
		}
	}

	~Bakery()
	{
		delete enterCont;
		delete ticketCont;
	}

	void lock(int threadId)
	{
		enterCont[threadId] = true;
		{
			ticketCont[threadId] = [this]() noexcept ->int
			{
				int max = ticketCont[0];
				for (int i = 0; i < threadNum; ++i)
				{
					if (ticketCont[i] > max)
					{
						max = ticketCont[i];
					}
				}
				return max + 1;
			}();
		}
		enterCont[threadId] = false;

		for (int i = 0; i < threadNum; ++i)
		{
			if (threadId == i) { continue; }
			while (enterCont[i]) {};
			while ((ticketCont[i]) && ((ticketCont[i], i) < (ticketCont[threadId], threadId)));
		}
	}

	void unlock(int threadId)
	{
		ticketCont[threadId] = 0;
	}
};

namespace ATOMIC_UTIL
{
	template <class TYPE> bool T_CAS(std::atomic<TYPE>* addr, TYPE oldValue, TYPE newValue) noexcept
	{
		return atomic_compare_exchange_strong(addr, &oldValue, newValue);
	};

	template <class TYPE> bool T_CAS(volatile TYPE* addr, TYPE oldValue, TYPE newValue) noexcept
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<TYPE>*>(addr), &oldValue, newValue);
	};
}

namespace GLOBAL
{
	constexpr static int LOOP_COUNT = 50000000;
	constexpr static int THREAD_NUMBER_TYPE = 5;

	constexpr static int ADD_VALUE = 2;

	//volatile int globalSum = 0;
	//std::atomic<int> globalSum = 0;
	int globalSum = 0;

	//std::atomic<bool> globalFlag = false;
	bool globalFlag = false;

	std::mutex globalLock;


	Bakery* bakeryLock = nullptr;
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
			std::atomic_thread_fence(std::memory_order_seq_cst);
			globalSum += 2;
			std::atomic_thread_fence(std::memory_order_seq_cst);
			bakeryLock->unlock(threadID);	// ------------------0
		}
	}
}

namespace ATOMIC_LOCK
{
	void Lock()
	{
		while (!ATOMIC_UTIL::T_CAS(&globalFlag, false, true));
	};

	void UnLock() { globalFlag = false; };

	void DoSum(int threadNum)
	{
		for (int i = 0, localLoopCount = LOOP_COUNT / threadNum; i < localLoopCount; ++i)
		{
			Lock();
			globalSum += 2;
			UnLock();
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

	std::cout << "\n";

	for (int i = 0; i < THREAD_NUMBER_TYPE; ++i)
	{
		std::vector<std::thread> threadCont;
		threadCont.reserve(threadNumArr[i]);

		globalSum = 0;

		auto startTime = high_resolution_clock::now();
		for (int j = 0; j < threadNumArr[i]; ++j) { threadCont.emplace_back(ATOMIC_LOCK::DoSum, threadNumArr[i]); }
		for (auto& thread : threadCont) { thread.join(); }
		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "ATOMIC : Thread Count : " << threadNumArr[i] << ", Sum : " << globalSum << ", Time : " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
	}

	std::cout << "\n";

	for (int i = 0; i < THREAD_NUMBER_TYPE; ++i)
	{
		std::vector<std::thread> threadCont;
		threadCont.reserve(threadNumArr[i]);

		globalSum = 0;

		auto startTime = high_resolution_clock::now();
		for (int j = 0; j < threadNumArr[i]; ++j) { threadCont.emplace_back(NO_LOCK::DoSum, threadNumArr[i]); }
		for (auto& thread : threadCont) { thread.join(); }
		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "No Lock : Thread Count : " << threadNumArr[i] << ", Sum : " << globalSum << ", Time : " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
	}

	std::cout << "\n";

	for (int i = 0; i < THREAD_NUMBER_TYPE; ++i)
	{
		std::vector<std::thread> threadCont;
		threadCont.reserve(threadNumArr[i]);

		globalSum = 0;

		auto startTime = high_resolution_clock::now();
		for (int j = 0; j < threadNumArr[i]; ++j) { threadCont.emplace_back(MUTEX::DoSum, threadNumArr[i]); }
		for (auto& thread : threadCont) { thread.join(); }
		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "MUTEX : Thread Count : " << threadNumArr[i] << ", Sum : " << globalSum << ", Time : " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
	}

	std::cout << "\n";

	for (int i = 0; i < THREAD_NUMBER_TYPE; ++i)
	{
		delete bakeryLock;

		std::vector<std::thread> threadCont;
		threadCont.reserve(threadNumArr[i]);

		globalSum = 0;
		bakeryLock = new Bakery(threadNumArr[i]);

		auto startTime = high_resolution_clock::now();
		for (int j = 0; j < threadNumArr[i]; ++j) { threadCont.emplace_back(BAKERY::DoSum, threadNumArr[i], j); }
		for (auto& thread : threadCont) { thread.join(); }
		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "BAKERY : Thread Count : " << threadNumArr[i] << ", Sum : " << globalSum << ", Time : " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
	}


	std::system("PAUSE");
}
