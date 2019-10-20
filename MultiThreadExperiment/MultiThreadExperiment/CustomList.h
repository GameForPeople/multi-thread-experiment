#pragma once

/*
	���� ����ȭ()
		- ��ü�� �ϳ��� ������ ó������

	������ ����ȭ()
		- �� �������� ������ ó������
		- �̵� ��, �� Next���� ���� ���� Lock�� �ؾ��Ѵ�.

	��õ�� ����ȭ()
		- �̵��� ���� ����� �ʰ�
		- ������ �Ҷ� ��ٴ�.
*/

namespace USING
{
	using _KeyType = int;
}using namespace USING;

namespace GLOBAL
{

}

namespace LIST_0_COARSE_GRAINED_SYNC // ���� ����ȭ
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

namespace LIST_1_FINE_GRAINED_SYNC // ������ ����ȭ
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

namespace LIST_2_OPTIMISTIC_SYNC // ��õ�� ����ȭ
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

namespace LIST_3_LAZY_SYNC // ������ ����ȭ
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

namespace LIST_3_LAZY_SYNC_WithSharedPtr // ������ ����ȭ�� shared_ptr ���....! ������ 2�� �̻��϶��� ������ ��!
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
}