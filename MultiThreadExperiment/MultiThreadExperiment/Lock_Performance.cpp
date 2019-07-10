#include "stdafx.h"

#include "Lock_Performance.h"

namespace LOCK_PERFORMANCE
{
	/*
		WriteReadTest()
		 - 테스트한 모든결과에 있어 Mutex < Shared_Mutex의 성능이 우세했다.
		 - atomic_flag를 활용한 동기화 방식의 성능이 오바였다. (다른거에 비해 너무 느린데...?)
		 - 

		물론 동기화도 이상없었다.

		나는 과연 Mutex를 계속 써야하는가?
	*/

	void SimpleLockUnlockTest()
	{
		const long long loopSize = 200000000;

		//std::cout << "myLock의 사이즈는 : " << sizeof(mutex) << ", wrLock의 사이즈는 : " << sizeof(shared_mutex) << "\n"; // 의미없음.

#pragma region [ MUTEX ]
		{
			mutex myLock;

			auto startTime = high_resolution_clock::now();

			for (int i = 0; i < loopSize; ++i)
			{
				myLock.lock();
				//
				myLock.unlock();
			}

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "Mutex의 Lock(), UnLock() 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
		}
#pragma endregion

#pragma region [ CRITICAL_SECTION ]
		{
			CRITICAL_SECTION myCS;
			::InitializeCriticalSection(&myCS);

			auto startTime = high_resolution_clock::now();

			for (int i = 0; i < loopSize; ++i)
			{
				::EnterCriticalSection(&myCS);
				//
				::LeaveCriticalSection(&myCS);
			}

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "CRITICAL_SECTION의 Lock(), UnLock() 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

			::DeleteCriticalSection(&myCS);
		}
#pragma endregion

#pragma region [ SPIN Lock ]
		{
			std::atomic_flag lock = ATOMIC_FLAG_INIT;

			auto startTime = high_resolution_clock::now();

			for (int i = 0; i < loopSize; ++i)
			{
				while (lock.test_and_set(std::memory_order_acquire));	//lock
				//
				lock.clear(std::memory_order_release);	//unlock
			}

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "SpinLock의 Lock(), UnLock() 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
		}
#pragma endregion

#pragma region [ Shared_Mutex ]
		{
			shared_mutex wrLock;

			auto startTime = high_resolution_clock::now();

			for (int i = 0; i < loopSize; ++i)
			{
				wrLock.lock();
				//
				wrLock.unlock();
			}

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "Shared Mutex의 Lock(), UnLock() 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
		}
#pragma endregion

	}

	void WriteReadTest()
	{
		const long long loopSize = 10000000;

		std::cout << "Write Thread 개수 입력 : ";
		int writeThreadCount;
		std::cin >> writeThreadCount;

		std::cout << "Read Thread 개수 입력 : ";
		int readThreadCount;
		std::cin >> readThreadCount;

		std::cout << "\n\n";

#pragma region [ MUTEX ]
		{
			mutex myLock;
			int sumValue = 0;
			std::vector<std::thread> writeThreadCont (writeThreadCount);
			std::vector<std::thread> readThreadCont (readThreadCount);

			writeThreadCont.resize(writeThreadCount);
			readThreadCont.resize(readThreadCount);

			auto startTime = high_resolution_clock::now();

			for (auto& thread : writeThreadCont)
			{
				thread = std::thread
				( 
					[&]() 
					{ 
						for (int i = 0; i < loopSize; ++i) 
						{
							myLock.lock();
							++sumValue;
							myLock.unlock();
						} 
					}
				);
			}

			for (auto& thread : readThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							myLock.lock();
							int localValue = sumValue;
							myLock.unlock();
							localValue += 1;
						}
					}
				);
			}

			for (auto& thread : writeThreadCont) thread.join();
			for (auto& thread : readThreadCont) thread.join();

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "Mutex의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
			cout << "값은? " << sumValue << " 입네다. \n\n";
		}
#pragma endregion

#pragma region [ SHARED_MUTEX ]
		{
			shared_mutex myLock;
			int sumValue = 0;

			std::vector<std::thread> writeThreadCont(writeThreadCount);
			std::vector<std::thread> readThreadCont(readThreadCount);

			writeThreadCont.resize(writeThreadCount);
			readThreadCont.resize(readThreadCount);

			auto startTime = high_resolution_clock::now();

			for (auto& thread : writeThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							myLock.lock();
							++sumValue;
							myLock.unlock();
						}
					}
				);
			}

			for (auto& thread : readThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							myLock.lock_shared();
							int localValue = sumValue;
							myLock.unlock_shared();
							localValue += 1;
						}
					}
				);
			}

			for (auto& thread : writeThreadCont) thread.join();
			for (auto& thread : readThreadCont) thread.join();

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "shared_mutex의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
			cout << "값은? " << sumValue << " 입네다. \n\n";
		}
#pragma endregion

#pragma region [ ATOMIC_FLAG ]
		{
			atomic_flag myLock = ATOMIC_FLAG_INIT;
			int sumValue = 0;

			std::vector<std::thread> writeThreadCont(writeThreadCount);
			std::vector<std::thread> readThreadCont(readThreadCount);

			writeThreadCont.resize(writeThreadCount);
			readThreadCont.resize(readThreadCount);

			auto startTime = high_resolution_clock::now();

			for (auto& thread : writeThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							while (myLock.test_and_set(std::memory_order_acquire));	//lock
							++sumValue;
							myLock.clear(std::memory_order_release);	//unlock
						}
					}
				);
			}

			for (auto& thread : readThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							while (myLock.test_and_set(std::memory_order_acquire));	//lock
							int localValue = sumValue;
							myLock.clear(std::memory_order_release);	//unlock
							localValue += 1;
						}
					}
				);
			}

			for (auto& thread : writeThreadCont) thread.join();
			for (auto& thread : readThreadCont) thread.join();

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "ATOMIC_FLAG (SpinLock) 의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
			cout << "값은? " << sumValue << " 입네다. \n\n";
		}
#pragma endregion

#pragma region [ ATOMIC INT ]
		{
			atomic<int> sumValue = 0;

			std::vector<std::thread> writeThreadCont(writeThreadCount);
			std::vector<std::thread> readThreadCont(readThreadCount);

			writeThreadCont.resize(writeThreadCount);
			readThreadCont.resize(readThreadCount);

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

			for (auto& thread : readThreadCont)
			{
				thread = std::thread
				(
					[&]()
					{
						for (int i = 0; i < loopSize; ++i)
						{
							int localValue = sumValue;
							localValue += 1;
						}
					}
				);
			}

			for (auto& thread : writeThreadCont) thread.join();
			for (auto& thread : readThreadCont) thread.join();

			auto endTime = high_resolution_clock::now() - startTime;
			cout << "Atomic Int의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
			cout << "값은? " << sumValue << " 입네다. \n\n";
		}
#pragma endregion
	}

}
