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
#include "Mutex_VS_SharedMutex.h"
//#include "Mutex_VS_LockGuard_VS_UniqueLock.h"

int main()
{
	Mutex_VS_SharedMutex::WriteReadTest();

	system("Pause");
}