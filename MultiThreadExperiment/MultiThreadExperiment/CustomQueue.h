#pragma once

namespace USING
{
	using _KeyType = int;

#ifdef _WIN64
	using _PointerType = long long;	// x64 64bit
#else
	using _PointerType = int;	// x32 32bit
#endif

}using namespace USING;

namespace GLOBAL
{
	constexpr int NUM_TEST = 10000000;
	constexpr int KEY_RANGE = 1000;

	constexpr _PointerType REMOVED_MASK = 0x01;

#ifdef _WIN64
	constexpr _PointerType POINTER_MASK = 0xFFFFFFFFFFFFFFFE;
#else 
	constexpr _PointerType POINTER_MASK = 0xFFFFFFFE;
#endif
}

namespace ATOMIC_UTIL
{
	template <class TYPE> bool T_CAS(std::atomic<TYPE>* addr, TYPE oldValue, TYPE newValue) noexcept
	{
		return atomic_compare_exchange_strong(addr, &oldValue, newValue);
	};

	template <class TYPE> bool T_CAS(volatile TYPE* addr, TYPE oldValue, TYPE newValue) noexcept
	{
		return atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<TYPE>*>(addr), &oldValue, newValue);
	};
}

namespace QUEUE_0_COARSE_GRAINED_SYNC // 성긴 동기화
{
	class Node {
	private:
		_KeyType key;
		Node* next;
		friend class Queue;

	public:
		Node() noexcept;

		Node(const _KeyType keyValue);

		~Node() = default;
	};

	class Queue {
		Node head;
		Node* tail;
		std::mutex queueLock;

	public:
		Queue() noexcept;

		~Queue();

		void Init();

		void Display(const int inCount = 20);

		void Enq(const _KeyType key);

		Node* Deq();
	};
};
