/*
	Git Repo Link : https://github.com/GameForPeople/multi-thread-experiment

	솔루션 플랫폼		: x64
	Windows SDK 버전 	: 10.0.17134.0
	플랫폼 도구 집합 	: Visual Studio 2017 (v141)
	문자 집합 			: 멀티바이트 문자 집합 사용
	C++ 언어 표준 		: ISO C++17 표준(/std:c++17)
	미리 컴파일된 헤더 	: stdafx.h
*/

#include "stdafx.h"
#include "Mutex_VS_SharedMutex.h"

int main()
{
	Mutex_VS_SharedMutex::TestFunc();

	system("Pause");
}