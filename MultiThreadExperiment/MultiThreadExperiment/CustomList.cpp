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

	LIST::LIST() noexcept
		: head(0x80000000)
		, tail(0x7FFFFFFF)
	{
		head.next = &tail;
		Init();
	}

	LIST::~LIST()
	{
		Init();
	}

	void LIST::Init()
	{
		Node* ptr;

		while (head.next != &tail)
		{
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	void LIST::Display(const int inCount)
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

	bool LIST::Add(const _KeyType key)
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

	bool LIST::Remove(const _KeyType key)
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

	bool LIST::Contains(const _KeyType key)
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

	LIST::LIST() noexcept
		: head(INT_MIN)
		, tail(INT_MAX)
	{
		head.next = &tail;
	}

	LIST::~LIST()
	{
		Init();
	}

	void LIST::Init()
	{
		Node* ptr;

		while (head.next != &tail)
		{
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}

	bool LIST::Add(const _KeyType key)
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

	bool LIST::Remove(const _KeyType key)
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

	bool LIST::Contains(const _KeyType key)
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