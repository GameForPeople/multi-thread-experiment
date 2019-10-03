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
}using namespace USING;

namespace GLOBAL
{

}

namespace GLOBAL_LIST_FUNCTION
{
	template <typename _List, typename _Node>
	void Display(const _List& list, const int inCount = 20)
	{
		_Node* ptr = list.head.next;
		int count = inCount;

		while (ptr != &list.tail)
		{
			std::cout << inCount - count << ": " << ptr->key << " , ";
			ptr = ptr->next;

			if (--count <= 0) break;
		}
	}
};

namespace LIST_0_COARSE_GRAINED_SYNC // 성긴 동기화
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

namespace LIST_1_FINE_GRAINED_SYNC // 세밀한 동기화
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

		bool Add(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			head.nodeLock.lock();					// +++ 1
			pred = &head;

			curr = pred->next;
			curr->nodeLock.lock();					// +++ 2

			while (curr->key < key)					// 2
			{
				pred->nodeLock.unlock();			// --- 1
				pred = curr;
				curr = curr->next;
				curr->nodeLock.lock();				// +++ 2
			}

			if (key == curr->key)					// 2
			{ 
				curr->nodeLock.lock();				// --- 1
				pred->nodeLock.unlock();			// --- 0
				return false; 
			}
			else									// 2
			{
				Node* tempNode = new Node(key);
				tempNode->next = curr;
				pred->next = tempNode;
				curr->nodeLock.lock();				// --- 1
				pred->nodeLock.unlock();			// --- 0
				return true;
			}
		}

		bool Remove(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			head.nodeLock.lock();					// +++ 1
			pred = &head;
			curr = pred->next;
			curr->nodeLock.lock();					// +++ 2

			while (curr->key < key)
			{
				pred->nodeLock.unlock();			// --- 1
				pred = curr;
				curr = curr->next;					
				curr->nodeLock.lock();				// +++ 2
			}

			if (key == curr->key)					// 2
			{
				pred->next = curr->next;
				delete curr;

				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return true;
			}
			else									// 2
			{
				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return false;
			}
		}

		bool Contains(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			head.nodeLock.lock();					// +++ 1
			pred = &head;
			curr = pred->next;
			curr->nodeLock.lock();					// +++ 2

			while (curr->key < key)
			{
				pred->nodeLock.unlock();			// --- 1
				pred = curr;
				curr = curr->next;
				curr->nodeLock.lock();				// +++ 2
			}

			if (key == curr->key) 
			{
				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return true;
			}
			else 
			{
				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return false;
			}
		}
	};
}

namespace LIST_1_FINE_GRAINED_SYNC_WithSmartLock // 세밀한 동기화
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

		bool Add(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			std::unique_lock<std::mutex> predLock(head.nodeLock, std::deff)
			head.nodeLock.lock();					// +++ 1
			pred = &head;

			curr = pred->next;
			curr->nodeLock.lock();					// +++ 2

			while (curr->key < key)					// 2
			{
				pred->nodeLock.unlock();			// --- 1
				pred = curr;
				curr = curr->next;
				curr->nodeLock.lock();				// +++ 2
			}

			if (key == curr->key)					// 2
			{
				curr->nodeLock.lock();				// --- 1
				pred->nodeLock.unlock();			// --- 0
				return false;
			}
			else									// 2
			{
				Node* tempNode = new Node(key);
				tempNode->next = curr;
				pred->next = tempNode;
				curr->nodeLock.lock();				// --- 1
				pred->nodeLock.unlock();			// --- 0
				return true;
			}
		}

		bool Remove(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			head.nodeLock.lock();					// +++ 1
			pred = &head;
			curr = pred->next;
			curr->nodeLock.lock();					// +++ 2

			while (curr->key < key)
			{
				pred->nodeLock.unlock();			// --- 1
				pred = curr;
				curr = curr->next;
				curr->nodeLock.lock();				// +++ 2
			}

			if (key == curr->key)					// 2
			{
				pred->next = curr->next;
				delete curr;

				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return true;
			}
			else									// 2
			{
				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return false;
			}
		}

		bool Contains(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			head.nodeLock.lock();					// +++ 1
			pred = &head;
			curr = pred->next;
			curr->nodeLock.lock();					// +++ 2

			while (curr->key < key)
			{
				pred->nodeLock.unlock();			// --- 1
				pred = curr;
				curr = curr->next;
				curr->nodeLock.lock();				// +++ 2
			}

			if (key == curr->key)
			{
				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return true;
			}
			else
			{
				pred->nodeLock.unlock();			// --- 1
				curr->nodeLock.unlock();			// --- 0
				return false;
			}
		}
	};
}

namespace LIST_2_OPTIMISTIC_SYNC // 낙천적 동기화
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

namespace LIST_3_LAZY_SYNC // 게으른 동기화
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