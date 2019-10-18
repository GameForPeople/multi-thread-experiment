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
		friend class LIST;

	public:
		Node() noexcept;

		Node(const _KeyType keyValue) noexcept;

		~Node() = default;
	};

	class LIST {
		Node head, tail;
		std::mutex listLock;

	public:
		LIST() noexcept;

		~LIST();

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

	class LIST {
	public:
		Node head, tail;

		LIST() noexcept;

		~LIST();

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

		Node() noexcept
			: key()
			, next(nullptr)
			, nodeLock()
		{
		}

		Node(const _KeyType keyValue) noexcept
			: key(keyValue)
			, next(nullptr)
			, nodeLock()
		{
		}

		~Node() {}
	};

	class LIST {
	public:
		Node head, tail;

		LIST() noexcept
			: head(INT_MIN)
			, tail(INT_MAX)
		{
			head.next = &tail;
		}

		~LIST()
		{
			Init();
		}

		void Init()
		{
			Node* ptr;

			while (head.next != &tail)
			{
				ptr = head.next;
				head.next = head.next->next;
				delete ptr;
			}
		}

		bool Validate(const Node* const inPred, const Node* const inCurr)
		{
			Node* comp = &head;

			while (comp->key <= inPred->key)
			{
				if (comp == inPred) { return comp->next == inCurr; }
				comp = comp->next;
			}
			return false;
		}

		bool Add(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			pred = &head;
			curr = pred->next;

			while (curr->key < key)				
			{
				pred = curr;
				curr = curr->next;
			}

			{
				std::scoped_lock<std::mutex, std::mutex> lock(pred->nodeLock, curr->nodeLock);

				if (Validate(pred, curr)) 
				{
					if (key == curr->key)
					{
						return false;
					}
					else
					{
						Node* tempNode = new Node(key);
						tempNode->next = curr;
						pred->next = tempNode;
						return true;
					}
				}
				return false;
			}
		}

		bool Remove(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			pred = &head;
			curr = pred->next;

			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			{
				std::scoped_lock<std::mutex, std::mutex> lock(pred->nodeLock, curr->nodeLock);

				if (Validate(pred, curr))
				{
					if (key == curr->key)				
					{
						pred->next = curr->next;
						// delete curr; // memory Leak!
						return true;
					}
					else { return false; }
				}
				return false;
			}
		}

		bool Contains(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			pred = &head;
			curr = pred->next;

			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			{
				std::scoped_lock<std::mutex, std::mutex> lock(pred->nodeLock, curr->nodeLock);

				if (Validate(pred, curr))
				{
					return (key == curr->key)
						? true
						: false;
				}
				return false;
			}
		}
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

		Node() noexcept
			: key()
			, next(nullptr)
			, nodeLock()
			, marked(false)
		{
		}

		Node(const _KeyType keyValue) noexcept
			: key(keyValue)
			, next(nullptr)
			, nodeLock()
			, marked(false)
		{
		}

		~Node() {}
	};

	class LIST {
	public:
		Node head, tail;

		LIST() noexcept
			: head(INT_MIN)
			, tail(INT_MAX)
		{
			head.next = &tail;
		}

		~LIST()
		{
			Init();
		}

		void Init()
		{
			Node* ptr;

			while (head.next != &tail)
			{
				ptr = head.next;
				head.next = head.next->next;
				delete ptr;
			}
		}

		bool Validate(const Node* const pred, const Node* const curr)
		{
			return !pred->marked && !curr->marked && pred->next == curr;
		}

		bool Add(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			pred = &head;
			curr = pred->next;

			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			{
				std::scoped_lock<std::mutex, std::mutex> lock(pred->nodeLock, curr->nodeLock);

				if (Validate(pred, curr))
				{
					if (key == curr->key)
					{
						return false;
					}
					else
					{
						Node* tempNode = new Node(key);
						tempNode->next = curr;
						pred->next = tempNode;
						return true;
					}
				}
				return false;
			}
		}

		bool Remove(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			pred = &head;
			curr = pred->next;

			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			{
				std::scoped_lock<std::mutex, std::mutex> lock(pred->nodeLock, curr->nodeLock);

				if (Validate(pred, curr))
				{
					if (key == curr->key)
					{
						curr->marked = true;
						pred->next = curr->next;
						// delete curr; // memory Leak!
						return true;
					}
					else { return false; }
				}
				return false;
			}
		}

		bool Contains(const _KeyType key)
		{
			Node* curr = &head;
			while (curr->key < key) { curr = curr->next; }
			return curr->key == key && !curr->marked;
		}
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

		Node() noexcept
			: key()
			, next(nullptr)
			, nodeLock()
			, marked(false)
		{
		}

		Node(const _KeyType keyValue) noexcept
			: key(keyValue)
			, next(nullptr)
			, nodeLock()
			, marked(false)
		{
		}

		~Node() = default;
	};

	class LIST {
		using _Node = std::shared_ptr<Node>;

	public:
		_Node head;
		_Node tail;

		LIST() noexcept
			: head(make_shared<Node>(INT_MIN))
			, tail(make_shared<Node>(INT_MAX))
		{
			head->next = tail;
		}

		~LIST()
		{
			Init();
		}

		void Init()
		{
			head->next = tail;
			//�������� �˾Ƽ� �Ҹ��.
		}

		void Display()
		{
			_Node ptr = head->next;

			for (int i = 0; i < 20; ++i)
			{
				std::cout << " " << ptr->key << ",";
				ptr = ptr->next;

				if (ptr == tail) break;
			}

			std::cout << "\n";
		}

		bool Validate(const _Node& const pred, const _Node& const curr)
		{
			return !pred->marked && !curr->marked && pred->next == curr;
		}

		bool Add(const _KeyType key)
		{
			_Node pred{ nullptr };
			_Node curr{ nullptr };

			pred = head;		// head�� �ٲ��� �ʱ� ������ �������.
			curr = pred->next;	// shared_ptr�� ����� �������� �ʱ� ������, �׷��� 

			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			{
				std::scoped_lock<std::mutex, std::mutex> lock(pred->nodeLock, curr->nodeLock);

				if (Validate(pred, curr))
				{
					if (key == curr->key)
					{
						return false;
					}
					else
					{
						_Node tempNode = make_shared<Node>(key);
						tempNode->next = curr;
						pred->next = tempNode;
						return true;
					}
				}
				return false;
			}
		}

		bool Remove(const _KeyType key)
		{
			_Node pred{ nullptr };
			_Node curr{ nullptr };

			pred = head;
			curr = pred->next;

			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			{
				std::scoped_lock<std::mutex, std::mutex> lock(pred->nodeLock, curr->nodeLock);

				if (Validate(pred, curr))
				{
					if (key == curr->key)
					{
						curr->marked = true;
						pred->next = curr->next;
						// delete curr; // memory auto release!
						return true;
					}
					else { return false; }
				}
				return false;
			}
		}

		bool Contains(const _KeyType key)
		{
			_Node curr = head;
			while (curr->key < key) { curr = curr->next; }
			return curr->key == key && !curr->marked;
		}
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
	void Display<LIST_3_LAZY_SYNC_WithSharedPtr::LIST, shared_ptr<LIST_3_LAZY_SYNC_WithSharedPtr::Node>>
		(const LIST_3_LAZY_SYNC_WithSharedPtr::LIST& list, const int inCount)
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