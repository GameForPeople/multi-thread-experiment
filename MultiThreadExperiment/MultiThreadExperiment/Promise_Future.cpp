#include "stdafx.h"

#include "Promise_Future.h"

namespace Promise_Future
{
	namespace Simple
	{
		void Product(std::promise<int>&& intPromise, int a, int b)
		{
			intPromise.set_value(a*b);
		}

		void TestFunc()
		{
			int a{ 20 };
			int b{ 10 };

			std::promise<int> prodPromise;
			std::promise<int> divPromise;

			std::future<int> prodResult = prodPromise.get_future();
			std::future<int> divResult = divPromise.get_future();

			std::thread prodThread(Product, std::move(prodPromise), a, b);

			Div div;
			std::thread divThread(div, std::move(divPromise), a, b);

			std::cout << prodResult.get() << std::endl;
			std::cout << divResult.get() << std::endl;

			prodThread.join();
			divThread.join();
		}
	}
}