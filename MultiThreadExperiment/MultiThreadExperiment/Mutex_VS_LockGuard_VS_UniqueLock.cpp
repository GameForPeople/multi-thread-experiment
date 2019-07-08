#include "stdafx.h"

#include "Mutex_VS_LockGuard_VS_UniqueLock.h"

/*
	단순 Lock Unlock으로는 3개 모두 비슷한 시간을 가진다.
*/

void Mutex_VS_LockGuard_VS_UniqueLock::TestFunc()
{
	const long long loopSize = 200000000;

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

	{
		mutex myLock;

		auto startTime = high_resolution_clock::now();

		for (int i = 0; i < loopSize; ++i)
		{
			std::lock_guard<mutex> myLockGuard(myLock);
		}

		auto endTime = high_resolution_clock::now() - startTime;
		cout << "lock_guard의 Lock(), UnLock() 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
	}

	{
		mutex myLock;

		auto startTime = high_resolution_clock::now();

		for (int i = 0; i < loopSize; ++i)
		{
			std::unique_lock<mutex> myUniqueLock(myLock);
		}

		auto endTime = high_resolution_clock::now() - startTime;
		cout << "myUniqueLock의 Lock(), UnLock() 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";
	}

}