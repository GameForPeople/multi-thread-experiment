#pragma once

namespace LOCK_VS_LOCKFREECONT
{
	struct Node
	{
		inline static constexpr int BUFFER_SIZE = 1000;
		int buffer[BUFFER_SIZE];
	};

	void TestFunc();
	void DoSomething();
}