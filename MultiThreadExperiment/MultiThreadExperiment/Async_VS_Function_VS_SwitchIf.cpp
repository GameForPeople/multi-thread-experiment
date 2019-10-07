#include "stdafx.h"

#include "Async_VS_Function_VS_SwitchIf.h"
#include <any>

namespace Async_VS_Function_VS_SwitchIf
{
	void TestFunc()
	{
		std::cout << "future�� ������� : " << sizeof(std::future<void>) << std::endl;
		auto lambdaFunction = [](int a, int b)->int {};
		std::cout << "lambdaFunction�� ������� : " << sizeof(lambdaFunction) << std::endl;
		
		auto asyncLazy = std::async(std::launch::deferred, [] {return std::chrono::system_clock::now(); });
		std::cout << "asyncLazy�� ������� : " << sizeof(asyncLazy) << std::endl;

		//std::function<void> functionPointer = []()->void {};
		
		std::cout << "lambdaFunction�� ������� : " << sizeof(std::any) << std::endl;

		//std::cout << "function�� ������� : " << sizeof(std::function<void>) << std::endl;
		//std::cout << "future�� ������� : " << sizeof(std::future<void>) << std::endl;
	}
}