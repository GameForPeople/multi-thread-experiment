#include "stdafx.h"

#include "CustomList.h"

namespace LIST_0_COARSE_GRAINED_SYNC {
	Node::Node() noexcept
		: key()
		, next(nullptr)
	{
	}

	Node::Node(const _KeyType keyValue) noexcept
		: key(keyValue)
		, next(nullptr)
	{
	}

	List::List() noexcept
		: head(0x80000000)
		, tail(0x7FFFFFFF)
	{
		head.next = &tail;
		Init();
	}

	List::~List()
	{
		Init();
	}

	void List::Init()
	{
		Node* ptr;

		while (head.next != &tail)
		{
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	void List::Display(const int inCount)
	{
		Node* ptr = head.next;
		int count = inCount;

		while (ptr != &tail)
		{
			std::cout << inCount - count << ": " << ptr->key << " , ";
			ptr = ptr->next;

			if (--count <= 0) break;
		}
	}

	bool List::Add(const _KeyType key)
	{
		Node* pred, * curr;
		pred = &head;

		{
			std::unique_lock<std::mutex> LocalLock(listLock);
			curr = pred->next;

			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			if (key == curr->key) { return false; }
			else
			{
				Node* node = new Node(key);
				node->next = curr;
				pred->next = node;
				return true;
			}
		}
	}

	bool List::Remove(const _KeyType key)
	{
		Node* pred, * curr;
		pred = &head;

		{
			std::unique_lock<std::mutex> LocalLock(listLock);
			curr = pred->next;
			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}

			if (key == curr->key)
			{
				pred->next = curr->next;
				delete curr;
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	bool List::Contains(const _KeyType key)
	{
		Node* pred, * curr;
		pred = &head;
		{
			std::unique_lock<std::mutex> LocalLock(listLock);
			curr = pred->next;
			while (curr->key < key)
			{
				pred = curr;
				curr = curr->next;
			}
			if (key == curr->key) { return true; }
			else { return false; }
		}
	}
}

namespace LIST_1_FINE_GRAINED_SYNC // 세밀한 동기화
{
	Node::Node() noexcept
		: key()
		, next(nullptr)
		, nodeLock()
	{
	}

	Node::Node(const _KeyType keyValue) noexcept
		: key(keyValue)
		, next(nullptr)
		, nodeLock()
	{
	}

	List::List() noexcept
		: head(INT_MIN)
		, tail(INT_MAX)
	{
		head.next = &tail;
	}

	List::~List()
	{
		Init();
	}

	void List::Init()
	{
		Node* ptr;

		while (head.next != &tail)
		{
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	bool List::Add(const _KeyType key)
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
			curr->nodeLock.unlock();			// --- 1
			pred->nodeLock.unlock();			// --- 0
			return false;
		}
		else									// 2
		{
			Node* tempNode = new Node(key);
			tempNode->next = curr;
			pred->next = tempNode;
			curr->nodeLock.unlock();			// --- 1
			pred->nodeLock.unlock();			// --- 0
			return true;
		}
	}

	bool List::Remove(const _KeyType key)
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
			curr->nodeLock.unlock();			// --- 1
			delete curr;

			pred->nodeLock.unlock();			// --- 0
			return true;
		}
		else									// 2
		{
			curr->nodeLock.unlock();			// --- 1
			pred->nodeLock.unlock();			// --- 0
			return false;
		}
	}

	bool List::Contains(const _KeyType key)
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
			curr->nodeLock.unlock();			// --- 1
			pred->nodeLock.unlock();			// --- 0
			return true;
		}
		else
		{
			curr->nodeLock.unlock();			// --- 1
			pred->nodeLock.unlock();			// --- 0
			return false;
		}
	}
}

namespace LIST_2_OPTIMISTIC_SYNC
{
	Node::Node() noexcept
		: key()
		, next(nullptr)
		, nodeLock()
	{
	}

	Node::Node(const _KeyType keyValue) noexcept
		: key(keyValue)
		, next(nullptr)
		, nodeLock()
	{
	}

	List::List() noexcept
		: head(INT_MIN)
		, tail(INT_MAX)
	{
		head.next = &tail;
	}

	List::~List()
	{
		Init();
	}

	void List::Init()
	{
		Node* ptr;

		while (head.next != &tail)
		{
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	bool List::Validate(const Node* const inPred, const Node* const inCurr)
	{
		Node* comp = &head;

		while (comp->key <= inPred->key)
		{
			if (comp == inPred) { return comp->next == inCurr; }
			comp = comp->next;
		}
		return false;
	}

	bool List::Add(const _KeyType key)
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

	bool List::Remove(const _KeyType key)
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

	bool List::Contains(const _KeyType key)
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
}

namespace LIST_3_LAZY_SYNC
{
	Node::Node() noexcept
		: key()
		, next(nullptr)
		, nodeLock()
		, marked(false)
	{
	}

	Node::Node(const _KeyType keyValue) noexcept
		: key(keyValue)
		, next(nullptr)
		, nodeLock()
		, marked(false)
	{
	}

	List::List() noexcept
		: head(INT_MIN)
		, tail(INT_MAX)
	{
		head.next = &tail;
	}

	List::~List()
	{
		Init();
	}

	void List::Init()
	{
		Node* ptr;

		while (head.next != &tail)
		{
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	bool List::Validate(const Node* const pred, const Node* const curr)
	{
		return !pred->marked && !curr->marked&& pred->next == curr;
	}

	bool List::Add(const _KeyType key)
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

	bool List::Remove(const _KeyType key)
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

	bool List::Contains(const _KeyType key)
	{
		Node* curr = &head;
		while (curr->key < key) { curr = curr->next; }
		return curr->key == key && !curr->marked;
	}
}

namespace LIST_3_LAZY_SYNC_WithSharedPtr
{
	Node::Node() noexcept
		: key()
		, next(nullptr)
		, nodeLock()
		, marked(false)
	{
	}

	Node::Node(const _KeyType keyValue) noexcept
		: key(keyValue)
		, next(nullptr)
		, nodeLock()
		, marked(false)
	{
	}

	List::List() noexcept
		: head(make_shared<Node>(INT_MIN))
		, tail(make_shared<Node>(INT_MAX))
	{
		head->next = tail;
	}

	List::~List()
	{
		Init();
	}

	void List::Init()
	{
		head->next = tail;
		//나머지는 알아서 소멸됨.
	}

	void List::Display()
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

	bool List::Validate(const _Node& const pred, const _Node& const curr)
	{
		return !pred->marked && !curr->marked&& pred->next == curr;
	}

	bool List::Add(const _KeyType key)
	{
		_Node pred{ nullptr };
		_Node curr{ nullptr };

		pred = head;		// head는 바뀌지 않기 때문에 상관없음.
		curr = pred->next;	// shared_ptr의 복사는 안전하지 않기 때문에, 그래서 

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

	bool List::Remove(const _KeyType key)
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

	bool List::Contains(const _KeyType key)
	{
		_Node curr = head;
		while (curr->key < key) { curr = curr->next; }
		return curr->key == key && !curr->marked;
	}
}
