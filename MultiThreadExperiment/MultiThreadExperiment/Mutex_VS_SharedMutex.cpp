#include "stdafx.h"

#include "Mutex_VS_SharedMutex.h"

namespace Mutex_VS_SharedMutex
{
	/*
		Mutex와 SharedMutex의 용도는 다르지만, Shared_mutex의 단순 성능을 확인하기 위한 테스트.
	*/
	void TestFunc()
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
}
