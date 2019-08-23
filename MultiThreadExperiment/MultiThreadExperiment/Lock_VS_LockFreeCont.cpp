#include "stdafx.h"

#include "Lock_VS_LockFreeCont.h"
#include <queue>

void LOCK_VS_LOCKFREECONT::TestFunc()
{
	const int NODE_COUNT = 10000;
	const int LOOP_COUNT = 1000000;
	const int FUTURE_COUNT = 4;

	{
		std::queue<Node*> nodeCont;
		std::vector<std::future<void>> futureCont;
		std::mutex simpleLock;

		for (int i = 0; i < NODE_COUNT; ++i) { nodeCont.emplace(new Node()); }

		futureCont.reserve(FUTURE_COUNT);

		auto startTime = high_resolution_clock::now();

		for (int i = 0; i < FUTURE_COUNT; ++i)
		{ 
			futureCont.push_back
			( 
				std::async
				(
					std::launch::async, [=, &nodeCont, &simpleLock]() noexcept -> void
					{
						for (int i = 0 ; i < LOOP_COUNT; ++i)
						{
							Node* pBuffer{ nullptr };
							{
								std::unique_lock<std::mutex> uniqueLock(simpleLock);
								pBuffer = nodeCont.front();
								nodeCont.pop();
							}

							std::this_thread::sleep_for(0ns);

							{
								std::unique_lock<std::mutex> uniqueLock(simpleLock);
								nodeCont.push(pBuffer);
							}
						}
					}
				)
			); 
		}

		
		for (auto& iter : futureCont) { /* iter.wait(); */ iter.get(); }

		auto endTime = high_resolution_clock::now() - startTime;
		cout << "Mutex Lock의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
		cout << "크기는? " << nodeCont.size() << "\n";

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
							while (!nodeCont.try_pop(pBuffer)) {}

							std::this_thread::sleep_for(0ns);

							nodeCont.push(pBuffer);
						}
					}
				)
			);
		}

		for (auto& iter : futureCont) { /* iter.wait(); */ iter.get(); }

		auto endTime = high_resolution_clock::now() - startTime;
		cout << "Lockfree Cont의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
		cout << "크기는? " << nodeCont.unsafe_size() << "\n";

		{
			Node* pBuffer{ nullptr };
			while (nodeCont.try_pop(pBuffer)) { delete pBuffer; }
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
}