/*
	Copyright 2020, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

// C++
#include <iostream>
#include <chrono>
#include <string>
#include <functional>
#include <typeinfo>

// C++ Container
#include <vector>
#include <map>
#include <tuple>

// C++ Multi-Thread
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>

// Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Macro
#define DO_PERFORMANCE() \
		DoPerformanceTest( noLock,           testTask, initTask, resultTask );	\
		DoPerformanceTest( mutex,            testTask, initTask, resultTask );	\
		DoPerformanceTest( sharedMutex,      testTask, initTask, resultTask );	\
		DoPerformanceTest( criticalSection,  testTask, initTask, resultTask );	\
		DoPerformanceTest( spinLock_BUSY,    testTask, initTask, resultTask );	\
		DoPerformanceTest( spinLock_TAS,     testTask, initTask, resultTask );	\
		DoPerformanceTest( spinLock_TTAS,    testTask, initTask, resultTask );	\
		DoPerformanceTest( spinLock_BACKOFF, testTask, initTask, resultTask );	\

// using
using _TestFunc = std::function< void () >;

// global
static /*constexpr*/ auto MAX_LOOP_COUNT   = 10000000;
static /*constexpr*/ auto MAX_THREAD_COUNT = 32;

#pragma region [ Synchronization Objects ]
struct NoLock
{
	NoLock() = default;

	void lock()  {};
	void unlock(){};
};

struct Mutex
{
	std::mutex& m_lock;

	Mutex( std::mutex& _lock ) : m_lock( _lock ){};

	void lock()  { m_lock.lock();   };
	void unlock(){ m_lock.unlock(); };

	void lock_shared()  { lock();   };
	void unlock_shared(){ unlock(); };
};

struct SharedMutex
{
	std::shared_mutex& m_lock;

	SharedMutex( std::shared_mutex& _lock ) : m_lock( _lock ){};

	void lock()  { m_lock.lock();   };
	void unlock(){ m_lock.unlock(); };

	void lock_shared()  { m_lock.lock_shared();   };
	void unlock_shared(){ m_lock.unlock_shared(); };
};

struct CriticalSection
{
	CRITICAL_SECTION& m_lock;

	CriticalSection( CRITICAL_SECTION& _lock ) : m_lock( _lock ){};

	void lock()  { EnterCriticalSection( &m_lock ); };
	void unlock(){ LeaveCriticalSection( &m_lock ); };

	void lock_shared()  { lock();   };
	void unlock_shared(){ unlock(); };
};

struct SpinLock_BUSY
{
	std::atomic_bool& m_lock;

	SpinLock_BUSY( std::atomic_bool& _lock ) : m_lock( _lock ){};

	void lock()
	{
		while ( m_lock.exchange( true, std::memory_order::memory_order_acquire ) ) {}
	};
	void unlock(){ m_lock.store( false, std::memory_order::memory_order_release ); };

	void lock_shared()  { lock();   };
	void unlock_shared(){ unlock(); };
};

struct SpinLock_TAS
{
	std::atomic_bool& m_lock;

	SpinLock_TAS( std::atomic_bool& _lock ) : m_lock( _lock ){};

	void lock()
	{
		while ( m_lock.exchange( true, std::memory_order::memory_order_acquire ) ) { std::this_thread::yield(); }
	};
	void unlock(){ m_lock.store( false, std::memory_order::memory_order_release ); };

	void lock_shared()  { lock();   };
	void unlock_shared(){ unlock(); };
};

struct SpinLock_TTAS
{
	std::atomic_bool& m_lock;

	SpinLock_TTAS( std::atomic_bool& _lock ) : m_lock( _lock ){};

	void lock()
	{
		while ( m_lock )
		{
			std::this_thread::yield();
		};

		while ( m_lock.exchange( true, std::memory_order::memory_order_acquire ) ) { std::this_thread::yield(); }
	};
	void unlock(){ m_lock.store( false, std::memory_order::memory_order_release ); };

	void lock_shared()  { lock();   };
	void unlock_shared(){ unlock(); };
};

struct SpinLock_TTAS_BACKOFF
{
	std::atomic_bool& m_lock;

	SpinLock_TTAS_BACKOFF( std::atomic_bool& _lock ) : m_lock( _lock ){};

	const void Sleep() { std::this_thread::sleep_for( std::chrono::milliseconds( rand() % 10 + 1 ) ); }
	
	void lock()
	{
		using namespace std::chrono_literals;

		while ( m_lock ) { Sleep(); };

		while ( m_lock.exchange( true, std::memory_order::memory_order_acquire ) ) { Sleep(); }
	};
	void unlock(){ m_lock.store( false, std::memory_order::memory_order_release ); };

	void lock_shared()  { lock();   };
	void unlock_shared(){ unlock(); };
};
using SpinLock_BACKOFF = SpinLock_TTAS_BACKOFF;
#pragma endregion

int main()
{
	std::cout << "CPU Thread Count : " << std::thread::hardware_concurrency() << "\n\n\n";

	std::mutex        _mutex;
	std::shared_mutex _sharedMutex;
	CRITICAL_SECTION  _criticalSection;
	std::atomic<bool> _flag{ false };
	
	InitializeCriticalSection( &_criticalSection );

	NoLock           noLock;
	Mutex            mutex           ( _mutex           );
	SharedMutex      sharedMutex     ( _sharedMutex     );
	CriticalSection  criticalSection ( _criticalSection );
	SpinLock_BUSY    spinLock_BUSY   ( _flag            );
	SpinLock_TAS     spinLock_TAS    ( _flag            );
	SpinLock_TTAS    spinLock_TTAS   ( _flag            );
	SpinLock_BACKOFF spinLock_BACKOFF( _flag            );

	auto DoPerformanceTest = []( auto& lockObject, _TestFunc testFunc, _TestFunc initFunc, _TestFunc resultFunc ) -> void
	{
		std::cout << "[ " << typeid( lockObject ).name() << " ] \n";

		for ( int threadCount = 1; threadCount <= MAX_THREAD_COUNT; threadCount = threadCount * 2 )
		{
			initFunc();
			
			std::vector < std::thread > threadCont;
			threadCont.reserve( threadCount );

			const auto startTime = std::chrono::high_resolution_clock::now();

			for ( int i = 0; i < threadCount; ++i )
			{
				threadCont.emplace_back(
					[ &lockObject, &testFunc, threadCount ]()
					{
						for ( int loopCount = 0; loopCount < (MAX_LOOP_COUNT / threadCount) ; ++loopCount )
						{
							std::lock_guard< decltype(lockObject) > localLock( lockObject );

							testFunc();
						}
					} );
			}

			for ( auto& th : threadCont ) { th.join(); }
			const auto endTime = std::chrono::high_resolution_clock::now() - startTime;
			std::cout << "Thread : " << threadCount << " ,  Time : "<< std::chrono::duration_cast< std::chrono::milliseconds>(endTime).count() << " msecs";

			threadCont.clear();

			resultFunc();
		}

		std::cout << "\n\n";
	};

	{
		std::cout << " Task Void =================================== \n";
		auto testTask   = [](){};
		auto initTask   = [](){};
		auto resultTask = [](){ std::cout << "\n"; };
		DO_PERFORMANCE();
		std::cout << "\n\n\n";
	}

	// MAX_LOOP_COUNT *= 10;
	{
		std::cout << " Task1 =================================== \n";
		volatile int x = 0;
		auto testTask   = [ &x ](){ ++x; };
		auto initTask   = [ &x ](){ x = 0; };
		auto resultTask = [ &x ](){ std::cout << ",  result : " << x << "\n"; };
		DO_PERFORMANCE();
	}

	// MAX_LOOP_COUNT /= 10;
	{
		std::cout << " Task2 =================================== \n";
		std::map< int, char[100] > cont; 
		auto testTask = [ &x ]() { ++x; };
		auto initTask = [ &x ]() { x = 0; };
		auto resultTask = [ &x ]() { std::cout << ",  result : " << x << "\n"; };
		DO_PERFORMANCE();
	}

	DeleteCriticalSection( &_criticalSection );
	std::system("PAUSE");
}