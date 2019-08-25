#include "stdafx.h"

#include "Lock_VS_LockFreeCont.h"
#include <queue>

void LOCK_VS_LOCKFREECONT::DoSomething()
{
	// Thread 4, LoopCount 1000000

	// 아무것도 안할 때 
	// Mutex(200ms), SpinLock(600ms), LockFreeCont(300ms)

	// 0ns 
	// this_thread::sleep_for(0ns); // Mutex(1100ms), SpinLock(650ms), LockFreeCont(300ms)

	// 1ns 
	//this_thread::sleep_for(1ns); // Mutex(145000ms), SpinLock(82000ms), LockFreeCont(300ms)
}

void LOCK_VS_LOCKFREECONT::TestFunc()
{
	const int NODE_COUNT = 100;
	const int LOOP_COUNT = 1000000;
	const int FUTURE_COUNT = 4;

	std::cout << "Thread Count : " << FUTURE_COUNT << ", Loop Count : " << LOOP_COUNT << "\n\n";

	{
		std::queue<Node*> nodeCont;
		std::vector<std::future<void>> futureCont;
		std::mutex mutexLock;

		for (int i = 0; i < NODE_COUNT; ++i) { nodeCont.emplace(new Node()); }

		futureCont.reserve(FUTURE_COUNT);

		auto startTime = high_resolution_clock::now();

		for (int i = 0; i < FUTURE_COUNT; ++i)
		{
			futureCont.push_back
			(
				std::async
				(
					std::launch::async, [=, &nodeCont, &mutexLock]() noexcept -> void
					{
						for (int i = 0; i < LOOP_COUNT; ++i)
						{
							Node* pBuffer{ nullptr };
							{
								std::unique_lock<std::mutex> uniqueLock(mutexLock);
								pBuffer = nodeCont.front();
								nodeCont.pop();
							}

							LOCK_VS_LOCKFREECONT::DoSomething();

							{
								std::unique_lock<std::mutex> uniqueLock(mutexLock);
								nodeCont.push(pBuffer);
							}
						}
					}
				)
			);
		}

		for (auto& iter : futureCont) { /* iter.wait(); */ iter.get(); }

		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "Mutex의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

		{
			Node* pBuffer{ nullptr };
			while (nodeCont.size()) { pBuffer = nodeCont.front(); nodeCont.pop(); delete pBuffer; }
		}
	}

	{
		std::queue<Node*> nodeCont;
		std::vector<std::future<void>> futureCont;
		std::atomic_flag spinLock;

		for (int i = 0; i < NODE_COUNT; ++i) { nodeCont.emplace(new Node()); }

		futureCont.reserve(FUTURE_COUNT);

		auto startTime = high_resolution_clock::now();

		for (int i = 0; i < FUTURE_COUNT; ++i)
		{ 
			futureCont.push_back
			( 
				std::async
				(
					std::launch::async, [=, &nodeCont, &spinLock]() noexcept -> void
					{
						for (int i = 0 ; i < LOOP_COUNT; ++i)
						{
							Node* pBuffer{ nullptr };
							{
								while (spinLock.test_and_set(std::memory_order_acquire));
								pBuffer = nodeCont.front();
								nodeCont.pop();
								spinLock.clear(std::memory_order_release);
							}

							LOCK_VS_LOCKFREECONT::DoSomething();

							{
								while (spinLock.test_and_set(std::memory_order_acquire));
								nodeCont.push(pBuffer);
								spinLock.clear(std::memory_order_release);
							}
						}
					}
				)
			); 
		}

		for (auto& iter : futureCont) { /* iter.wait(); */ iter.get(); }

		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "Spin Lock의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

		{
			Node* pBuffer{ nullptr };
			while (nodeCont.size()) { pBuffer = nodeCont.front(); nodeCont.pop(); delete pBuffer; }
		}
	}

	{
		concurrency::concurrent_queue<Node*> nodeCont;
		std::vector<std::future<void>> futureCont;

		for (int i = 0; i < NODE_COUNT; ++i) { nodeCont.push(new Node()); }

		futureCont.reserve(FUTURE_COUNT);

		auto startTime = high_resolution_clock::now();

		for (int i = 0; i < FUTURE_COUNT; ++i)
		{
			futureCont.push_back
			(
				std::async
				(
					std::launch::async, [=, &nodeCont]() noexcept -> void
					{
						for (int i = 0; i < LOOP_COUNT; ++i)
						{
							Node* pBuffer{ nullptr };
							while (!nodeCont.try_pop(pBuffer))

							LOCK_VS_LOCKFREECONT::DoSomething();

							nodeCont.push(pBuffer);
						}
					}
				)
			);
		}

		for (auto& iter : futureCont) { /* iter.wait(); */ iter.get(); }

		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "Lockfree Cont의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

		{
			Node* pBuffer{ nullptr };
			while (nodeCont.try_pop(pBuffer)) { delete pBuffer; }
		}
	}
}

	/*
	{
		concurrency::concurrent_queue<Node*> nodeCont;
		std::vector<std::thread> threadCont;

		for (int i = 0; i < NODE_COUNT; ++i) { nodeCont.push(new Node()); }

		threadCont.reserve(FUTURE_COUNT);

		auto startTime = high_resolution_clock::now();

		for (int i = 0; i < FUTURE_COUNT; ++i)
		{
			threadCont.push_back
			(
				std::thread
				(
					[=, &nodeCont]() noexcept -> void
					{
						for (int i = 0; i < LOOP_COUNT; ++i)
						{
							Node* pBuffer{ nullptr };
							while (!nodeCont.try_pop(pBuffer)) {}
							
							std::this_thread::sleep_for(0ns);
							
							nodeCont.push(pBuffer);
						}
					}
				)
			);
		}

		for (auto& iter : threadCont) { iter.join(); }

		auto endTime = high_resolution_clock::now() - startTime;
		cout << "Lockfree Cont - thread 의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
		cout << "크기는? " << nodeCont.unsafe_size() << "\n";

		{
			Node* pBuffer{ nullptr };
			while (nodeCont.try_pop(pBuffer)) { delete pBuffer; }
		}
	}
	*/