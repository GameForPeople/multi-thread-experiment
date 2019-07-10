#include "stdafx.h"

#include "Atomic_Simple_Test.h"

namespace ATOMIC_SIMPLE_TEST
{
	void SimpleAddTest()
	{
		const long long loopSize = 20000000;

		{
			atomic<int> sumValue = 0;

			std::vector<std::thread> writeThreadCont;
			writeThreadCont.resize(8);

			auto startTime = high_resolution_clock::now();

			for (auto& thread : writeThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							++sumValue;
						}
					}
				);
			}

			for (auto& thread : writeThreadCont) thread.join();

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "++sumValue의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
			cout << "값은? " << sumValue << " 입네다. \n\n";
		}

		{
			atomic<int> sumValue = 0;

			std::vector<std::thread> writeThreadCont;
			writeThreadCont.resize(8);

			auto startTime = high_resolution_clock::now();

			for (auto& thread : writeThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							sumValue.fetch_add(1);
						}
					}
				);
			}

			for (auto& thread : writeThreadCont) thread.join();

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "fetch_add - seq_cst의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
			cout << "값은? " << sumValue << " 입네다. \n\n";
		}


		{
			atomic<int> sumValue = 0;

			std::vector<std::thread> writeThreadCont;
			writeThreadCont.resize(8);

			auto startTime = high_resolution_clock::now();

			for (auto& thread : writeThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							sumValue.fetch_add(1, std::memory_order_relaxed);
						}
					}
				);
			}

			for (auto& thread : writeThreadCont) thread.join();

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "fetch_add - memory_order_relaxed의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
			cout << "값은? " << sumValue << " 입네다. \n\n";
		}
	}

}