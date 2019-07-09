#include "stdafx.h"

#include "Shared_Mutex_WithGuard.h"

/*
	shared_lock에 RAII를 더하다.
		왜?
			- 실수 방지
			- 에러 시 release까지 못가고 다른데로 Jump하는데 이러면 Lock 못품. 이런 경우 방지.
			- 성능 걱정 X 거진비슷함.

	 관련KeyWord
		std::lock_guard<T>  - 고냥 mutex 같은거나 넣어서 고냥 lock할때 씀.
		std::unique_lock<T> - 저놈 상위호환. wrLock에서 wLock은 이걸로 구현
		std::shared_lock<T> - wrLock의 rLock은 이걸로 구현. 좋네.

		각각 관련해서 심화사항은 따로 공부해서 함 해봅세다.
*/
void Shared_Mutex_WithGuard::TestFunc()
{
	std::shared_mutex rootLock;
	std::vector<std::thread> writeThreadCont;
	std::vector<std::thread> readThreadCont;
	
	int globalSum{ 0 };
	const int loopSize { 10000000 };

	writeThreadCont.resize(2);
	readThreadCont.resize(4);

	for (auto& thread : writeThreadCont)
	{
		thread = std::thread
		([&] 
			{
				for (int i = 0; i < loopSize; ++i)
				{
					std::unique_lock<std::shared_mutex> localWriteLock(rootLock);
					++globalSum;
				}
			}
		);
	}

	for (auto& thread : readThreadCont)
	{
		thread = std::thread
		([&]
			{
				int tempLocal{};

				for (int i = 0; i < 100; ++i)
				{
					{
						std::shared_lock<std::shared_mutex> localWriteLock(rootLock);
						tempLocal = globalSum;
					}
					this_thread::sleep_for(std::chrono::milliseconds(50));
				}

				this_thread::sleep_for(std::chrono::seconds(1));

				{
					std::unique_lock<std::shared_mutex> localWriteLock(rootLock);
					std::cout << this_thread::get_id() << " : " << tempLocal << "\n";
				}
			}
		);
	}

	for (auto& thread : writeThreadCont) thread.join();
	for (auto& thread : readThreadCont) thread.join();
}