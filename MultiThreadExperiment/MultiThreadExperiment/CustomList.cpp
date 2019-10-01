#include "stdafx.h"

#include "CustomList.h"

namespace LIST_COARSE_GRAINED_SYNC {
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


