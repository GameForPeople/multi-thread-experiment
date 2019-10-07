#include "stdafx.h"

#include "Async_VS_Function_VS_SwitchIf.h"
#include <any>

namespace Async_VS_Function_VS_SwitchIf
{
	void TestFunc()
	{
		std::cout << "future의 사이즈는 : " << sizeof(std::future<void>) << std::endl;
		auto lambdaFunction = [](int a, int b)->int {};
		std::cout << "lambdaFunction의 사이즈는 : " << sizeof(lambdaFunction) << std::endl;
		
		auto asyncLazy = std::async(std::launch::deferred, [] {return std::chrono::system_clock::now(); });
		std::cout << "asyncLazy의 사이즈는 : " << sizeof(asyncLazy) << std::endl;

		//std::function<void> functionPointer = []()->void {};
		
		std::cout << "lambdaFunction의 사이즈는 : " << sizeof(std::any) << std::endl;

		//std::cout << "function의 사이즈는 : " << sizeof(std::function<void>) << std::endl;
		//std::cout << "future의 사이즈는 : " << sizeof(std::future<void>) << std::endl;
	}
}