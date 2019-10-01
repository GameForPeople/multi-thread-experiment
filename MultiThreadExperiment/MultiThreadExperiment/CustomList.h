#pragma once

/*
	성긴 동기화(CList)
		- 전체를 하나의 락으로 처리하자

	세밀한 동기화()
		- 각 노드단위를 락으로 처리하자
		- 이동 시, 즉 Next값을 읽을 때도 Lock을 해야한다.
*/

namespace USING
{
	using _KeyType = int;
}using namespace USING;

namespace LIST_COARSE_GRAINED_SYNC // 성긴 동기화
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
}

namespace LIST_FINE_GRAINED_SYNC // 세밀한 동기화
{
	class Node {
	public:
		_KeyType key;
		Node* next;
		std::mutex lock;

		Node() noexcept
			: key()
			, next(nullptr)
			, lock()
		{
		}

		Node(const _KeyType keyValue) noexcept
			: key(keyValue)
			, next(nullptr)
			, lock()
		{
		}

		~Node() {}
	};

	class CLIST {
		Node head, tail;

	public:
		CLIST() noexcept
			: head(0x80000000)
			, tail(0x7FFFFFFF)
		{
			head.next = &tail;
			Init();
		}

		~CLIST()
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

		void Display(const int inCount = 20)
		{
			Node* p = head.next;
			int count = inCount;

			while (p != &tail)
			{
				std::cout << inCount - count << ": " << p->key << " , ";
				p = p->next;

				if (--count <= 0) break;
			}
		}

		bool Add(const _KeyType key)
		{
			Node* pred, * curr;
			pred = &head;

			pred->lock.lock();
			curr = pred->next;
			pred->lock.unlock();

			curr->lock.lock();

			while (curr->key < key)
			{
				pred->lock.unlock();
				pred = curr;

				curr->next->lock.lock();
				curr = curr->next;
			}

			if (key == curr->key) { pred->lock.unlock(); curr->lock.unlock(); return false; }
			else
			{
				Node* node = new Node(key);
				node->next = curr;
				pred->next = node;
				pred->lock.unlock();
				curr->lock.unlock();
				return true;
			}
		}

		bool Remove(const _KeyType key)
		{
			Node* pred, * curr;
			pred = &head;

			{
				pred->lock.lock();
				curr = pred->next;
				pred->lock.unlock();

				curr->lock.lock();

				while (curr->key < key)
				{
					pred->lock.unlock();
					pred = curr;

					curr->next->lock.lock();
					curr = curr->next;
				}

				if (key == curr->key)
				{
					curr->next->lock.lock();
					pred->next = curr->next;
					curr->next->lock.unlock();
					pred->lock.unlock();

					std::mutex& tempLock = curr->lock;
					delete curr;
					tempLock.unlock();
					return true;
				}
				else
				{
					pred->lock.unlock();
					curr->lock.unlock();
					return false;
				}
			}
		}

		bool Contains(const _KeyType key)
		{
			Node* pred, * curr;
			pred = &head;
			{
				pred->lock.lock();
				curr = pred->next;
				pred->lock.unlock();

				curr->lock.lock();
				while (curr->key < key)
				{
					pred = curr;
					curr->next->lock.lock();
					curr = curr->next;
				}
				if (key == curr->key) {
					curr->lock.unlock();
					return true;
				}
				else {
					curr->lock.unlock();
					return false;
				}
			}
		}
	};
}