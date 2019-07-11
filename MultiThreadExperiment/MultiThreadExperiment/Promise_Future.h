#pragma once

namespace Promise_Future
{
	namespace Simple
	{
		void Product(std::promise<int>&& intPromise, int a, int b);
		struct Div 
		{
			void operator() (std::promise<int>&& intPromise, int a, int b) const
			{
				intPromise.set_value(a / b);
			}
		};
		void TestFunc();
	}
}