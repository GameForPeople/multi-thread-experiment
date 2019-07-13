#include "stdafx.h"

#include "Async.h"

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
		std::async(std::launch::async, []() 
			{
				std::this_thread::sleep_for(std::chrono::seconds(2));
				std::cout << "First Thread Fire!" << std::endl;
			}
		);

		std::async(std::launch::async, []()
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << "Second Thread Fire!" << std::endl;
			}
		);

		std::cout << "Main Thread Fire!" << std::endl;
	}
}