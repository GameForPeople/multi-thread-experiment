#pragma once

/*
	성긴 동기화()
		- 전체를 하나의 락으로 처리하자

	세밀한 동기화()
		- 각 노드단위를 락으로 처리하자
		- 이동 시, 즉 Next값을 읽을 때도 Lock을 해야한다.

	낙천적 동기화()
		- 이동할 때는 잠그지 않고
		- 수정을 할때 잠근다.
*/

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
	constexpr int NUM_TEST = 4000000;
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

namespace LIST_0_COARSE_GRAINED_SYNC // 성긴 동기화
{
	class Node {
	private:
		_KeyType key;
		Node* next;
		friend class List;

	public:
		Node() noexcept;

		Node(const _KeyType keyValue) noexcept;

		~Node() = default;
	};

	class List {
		Node head, tail;
		std::mutex listLock;

	public:
		List() noexcept;

		~List();

		void Init();

		void Display(const int inCount = 20);

		bool Add(const _KeyType key);

		bool Remove(const _KeyType key);

		bool Contains(const _KeyType key);
	};
};

namespace LIST_1_FINE_GRAINED_SYNC // 세밀한 동기화
{
	class Node {
	public:
		_KeyType key;
		Node* next;
		std::mutex nodeLock;

		Node() noexcept;

		Node(const _KeyType keyValue) noexcept;

		~Node() = default;
	};

	class List {
	public:
		Node head, tail;

		List() noexcept;

		~List();

		void Init();

		bool Add(const _KeyType key);

		bool Remove(const _KeyType key);

		bool Contains(const _KeyType key);
	};
}

namespace LIST_2_OPTIMISTIC_SYNC // 낙천적 동기화
{
	class Node {
	public:
		_KeyType key;
		Node* next;
		std::mutex nodeLock;

		Node() noexcept;

		Node(const _KeyType keyValue) noexcept;

		~Node() = default;
	};

	class List {
	public:
		Node head, tail;

		List() noexcept;

		~List();

		void Init();

		bool Validate(const Node* const inPred, const Node* const inCurr);

		bool Add(const _KeyType key);

		bool Remove(const _KeyType key);

		bool Contains(const _KeyType key);
	};
}

namespace LIST_3_LAZY_SYNC // 게으른 동기화
{
	class Node {
	public:
		_KeyType key;
		Node* next;
		std::mutex nodeLock;
		bool marked;

		Node() noexcept;

		Node(const _KeyType keyValue) noexcept;

		~Node() = default;
	};

	class List {
	public:
		Node head, tail;

		List() noexcept;

		~List();

		void Init();

		bool Validate(const Node* const pred, const Node* const curr);

		bool Add(const _KeyType key);

		bool Remove(const _KeyType key);

		bool Contains(const _KeyType key);
	};
}

namespace LIST_3_LAZY_SYNC_WithSharedPtr // 게으른 동기화와 shared_ptr 사용....! 쓰레드 2개 이상일때는 문제가 됨!
{
	class Node {
	public:
		_KeyType key;
		std::shared_ptr<Node> next;
		std::mutex nodeLock;
		bool marked;

		Node() noexcept;

		Node(const _KeyType keyValue) noexcept;

		~Node() = default;
	};

	class List {
		using _Node = std::shared_ptr<Node>;

	public:
		_Node head;
		_Node tail;

		List() noexcept;

		~List();

		void Init();

		void Display();

		bool Validate(const _Node& const pred, const _Node& const curr);

		bool Add(const _KeyType key);

		bool Remove(const _KeyType key);

		bool Contains(const _KeyType key);
	};
}

namespace LIST_4_LOCKFREE // 락 프리.
{
	class Node;

	class MarkedPointer
	{
		_PointerType value;	// next Node Pointer with removed Mark(1bit).
	public:
		MarkedPointer() noexcept;

		void Set(const Node* const node, const bool removed) noexcept;

		_NODISCARD Node* GetPtr() const;

		_NODISCARD Node* GetPtrWithRemoved(bool& removed) const;

		bool CAS(const Node* const oldNode, const Node* const newNode, const bool oldRemoved, const bool newRemoved);

		bool TryMark(const Node* const oldNode, const bool newMark);
	};

	class Node
	{
	public:
		_KeyType key;
		MarkedPointer next;

		Node() noexcept;
		Node(const _KeyType keyValue) noexcept;
		~Node() = default;
	};

	class List {
	public:
		Node head, tail;
		concurrent_queue<Node*> memoryPool;

		List(const int memoryPoolSize /* = GLOBAL::KEY_RANGE * 2 */) noexcept;

		~List();

		void Init();

		// Validate를 따로하지않음 의미가없음... 내가 락을 걸어놨으니까 Validate하는 건데, 락을 걸지 않는 락프리 자료구조에서는 의미가없음.
		//bool Validate(const Node* const pred, const Node* const curr)
		//{
		//	return !pred->marked && !curr->marked && pred->next == curr;
		//}

		void Find(const _KeyType key, Node* (&pred), Node* (&curr));

		bool Add(const _KeyType key);

		bool Remove(const _KeyType key);

		bool Contains(const _KeyType key);
	};
}

namespace CUSTOM_LIST
{
	template <typename _List, typename _Node>
	void Display(const _List& list, const int inCount = 20)
	{
		_Node* ptr = list.head.next;

		for (int i = 0; i < inCount; ++i)
		{
			std::cout << " " << ptr->key << ",";
			ptr = ptr->next;

			if (ptr == &list.tail) break;
		}

		std::cout << "\n";
	}

	template <>
	void Display<LIST_3_LAZY_SYNC_WithSharedPtr::List, LIST_3_LAZY_SYNC_WithSharedPtr::Node>
		(const LIST_3_LAZY_SYNC_WithSharedPtr::List& list, const int inCount)
	{
		shared_ptr<LIST_3_LAZY_SYNC_WithSharedPtr::Node> ptr = list.head->next;

		for (int i = 0; i < inCount; ++i)
		{
			std::cout << " " << ptr->key << ",";
			ptr = ptr->next;

			if (ptr == list.tail) break;
		}

		std::cout << "\n";
	}

	template <>
	void Display<LIST_4_LOCKFREE::List, LIST_4_LOCKFREE::Node>(const LIST_4_LOCKFREE::List& list, const int inCount)
	{
		LIST_4_LOCKFREE::Node* ptr = list.head.next.GetPtr();

		for (int i = 0; i < inCount; ++i)
		{
			if (ptr == &list.tail) break;

			std::cout << " " << ptr->key << ",";
			ptr = ptr->next.GetPtr();
		}

		std::cout << "\n";
	}
}