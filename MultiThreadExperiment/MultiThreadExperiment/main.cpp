/*
	Git Repo Link : https://github.com/GameForPeople/multi-thread-experiment

	�ַ�� �÷���		: x64
	Windows SDK ���� 	: 10.0.17134.0
	�÷��� ���� ���� 	: Visual Studio 2017 (v141)
	���� ���� 			: ��Ƽ����Ʈ ���� ���� ���
	C++ ��� ǥ�� 		: ISO C++17 ǥ��(/std:c++17)
	�̸� �����ϵ� ��� 	: stdafx.h
*/

#include "stdafx.h"
//#include "Mutex_VS_SharedMutex.h"
//#include "Mutex_VS_LockGuard_VS_UniqueLock.h"
#include "Shared_Mutex_WithGuard.h"
#include "Atomic_Simple_Test.h"
#include "Async.h"
#include "Promise_Future.h"
#include "Lock_VS_LockFreeCont.h"
#include "Async_VS_Function_VS_SwitchIf.h"

//int main()
//{
//	//LOCK_VS_LOCKFREECONT::TestFunc();
//	//ASYNC::FireForget();
//	//Async_VS_Function_VS_SwitchIf::TestFunc();
//	//Promise_Future::Simple::TestFunc();
//	std::cout << GetTickCount64() << std::endl;
//	this_thread::sleep_for(1.5s);
//	std::cout << GetTickCount64() << std::endl;
//
//	system("Pause");
//}

void Func()
{
	int a{ 0 };
	int* b{ nullptr };
	int c[100]{ 0, };
	int d{ 0 };
	int* e{ nullptr };
	int f[100]{ 0, };

	std::cout 
		<< "a : " << &a 
		<< ", b : " << &b 
		<< ", c : " << &c 
		<< ", d : " << &d 
		<< ", e : " << &e
		<< ", f : " << &f
		<< "\n";

	//std::atomic_thread_fence(std::memory_order_seq_cst);

	f[-1] = 100;	// d
	f[101] = 100;	// f[0]

	std::cout 
		<< ", d : " << d
		<< ", f : " << f[0]
		<< "\n";
}

//int main()
//{
//	system("Pause");
//}
