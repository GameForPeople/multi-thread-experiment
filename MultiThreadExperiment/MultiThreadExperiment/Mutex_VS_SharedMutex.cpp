#include "stdafx.h"

#include "Mutex_VS_SharedMutex.h"

namespace Mutex_VS_SharedMutex
{
	/*
		Mutex�� SharedMutex�� �뵵�� �ٸ�����, Shared_mutex�� �ܼ� ������ Ȯ���ϱ� ���� �׽�Ʈ.
	*/
	void TestFunc()
	{
		const long long loopSize = 200000000;

		//std::cout << "myLock�� ������� : " << sizeof(mutex) << ", wrLock�� ������� : " << sizeof(shared_mutex) << "\n"; // �ǹ̾���.

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
			cout << "Mutex�� Lock(), UnLock() ������? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
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
			cout << "CRITICAL_SECTION�� Lock(), UnLock() ������? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

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
			cout << "SpinLock�� Lock(), UnLock() ������? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
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
			cout << "Shared Mutex�� Lock(), UnLock() ������? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
		}
#pragma endregion

	}
}
