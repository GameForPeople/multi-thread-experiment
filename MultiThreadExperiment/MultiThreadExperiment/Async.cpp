#include "stdafx.h"

#include "Async.h"

/*
	LazyAndEager
	std::launch::deferred ==> Get, Wait 불릴때 까지 계산 안하고 대기.
	std::launch::async ==> 멀티쓰레드로 바로 병렬 처리.

	FireForget
	async는 병렬로 처리되나, 소멸자가 불릴때 블로킹된다.
*/
namespace ASYNC
{
	void SimpleExample()
	{
		auto future = std::async([]()-> int { return 3000; });

		future.get();
	}

	void LazyAndEager()
	{
		auto startTime = std::chrono::system_clock::now();

		auto asyncLazy = std::async(std::launch::deferred, [] {return std::chrono::system_clock::now(); });
		auto asyncEager = std::async(std::launch::async, [] {return std::chrono::system_clock::now(); });

		std::this_thread::sleep_for(std::chrono::seconds(1));

		auto lazyTime =	std::chrono::duration<double>(asyncLazy.get() - startTime).count();
		auto eagerTime = std::chrono::duration<double>(asyncEager.get() - startTime).count();

		std::cout << "lazyTime : " << lazyTime << "\n";
		std::cout << "eagerTime : " << eagerTime;
	}

	void FireForget()
	{
		// R-Value Future
		std::async(std::launch::async, []() 
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << "First Thread Fire!" << std::endl;					// 1
			}
		);	

		{
			// L-Value Future & MultiThread
			auto tempFuture = std::async(std::launch::async, []()
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					std::cout << "Second Thread Fire!" << std::endl;			// 3
				}
			);

			std::cout << "First Reload!" << std::endl;							// 2
		}
		std::cout << "Second Reload!" << std::endl;								// 4

		{
			// L-Value Future & MultiThread with Condition
			std::atomic<bool> tempFlag = false;
			auto tempFuture = std::async(std::launch::async, [&tempFlag]()
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					std::cout << "Third Thread Fire!" << std::endl;				// 5
					tempFlag = true;
				}
			);

			while (!tempFlag) { std::cout << "."; std::this_thread::sleep_for(0.6s); };

			std::cout << "Third Reload!" << std::endl;							// 6
		}

		std::cout << "Main Thread Fire!" << std::endl;							// 7
	}
}