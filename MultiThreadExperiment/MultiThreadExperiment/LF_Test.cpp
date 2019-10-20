// cpp
#include <iostream>

// STL
#include <vector>

// C++ 11
#include <atomic>
#include <thread>
#include <mutex>

// c++17
#define _NODISCARD [[nodiscard]]

using namespace std;
using namespace std::chrono;

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

namespace LIST_4_LOCKFREE // 락 프리.
{
	class Node;

	class MarkedPointer
	{
		_PointerType value;	// next Node Pointer with removed Mark(1bit).
	public:
		MarkedPointer()
			: value()
		{
			Set(nullptr, false);
		};

		void Set(Node* node, bool removed)
		{
			value = reinterpret_cast<_PointerType>(node);

			value = removed
				? value | GLOBAL::REMOVED_MASK
				: value & GLOBAL::POINTER_MASK;
		}

		_NODISCARD Node* GetPtr() const
		{
			return reinterpret_cast<Node*>(value & GLOBAL::POINTER_MASK);
		}

		_NODISCARD Node* GetPtrWithRemoved(bool& removed)
		{
			auto temp = value;

			0 == (temp & GLOBAL::REMOVED_MASK)
				? removed = false
				: removed = true;

			return reinterpret_cast<Node*>(value & GLOBAL::POINTER_MASK);
		}

		bool CAS(Node* oldNode, Node* newNode, bool oldRemoved, bool newRemoved)
		{
			_PointerType oldValue = reinterpret_cast<_PointerType>(oldNode);

			if (oldRemoved) oldValue = oldValue | GLOBAL::REMOVED_MASK;
			else oldValue = oldValue & GLOBAL::POINTER_MASK;

			_PointerType newValue = reinterpret_cast<_PointerType>(newNode);
			if (newRemoved) newValue = newValue | GLOBAL::REMOVED_MASK;
			else newValue = newValue & GLOBAL::POINTER_MASK;

			return ATOMIC_UTIL::T_CAS(&value, oldValue, newValue);
		}

		bool TryMark(Node * oldNode, bool newMark)
		{
			auto oldValue = reinterpret_cast<_PointerType>(oldNode);
			auto newValue = oldValue;

			if (newMark) newValue = newValue | GLOBAL::REMOVED_MASK;
			else newValue = newValue & GLOBAL::POINTER_MASK;

			return ATOMIC_UTIL::T_CAS(&value, oldValue, newValue);
		}
	};

	class Node
	{
	public:
		_KeyType key;
		MarkedPointer next;

		Node() noexcept
			: key()
			, next()
		{
		}

		Node(const _KeyType keyValue) noexcept
			: key(keyValue)
			, next()
		{
		}

		~Node() = default;
	};

	class List {
	public:
		Node head, tail;

		List() noexcept
			: head(INT_MIN)
			, tail(INT_MAX)
		{
			head.next.Set(&tail, false);
		}

		~List()
		{
			Init();
		}

		void Init()
		{
			Node* ptr;

			while (head.next.GetPtr() != &tail)
			{
				ptr = head.next.GetPtr();
				head.next = head.next.GetPtr()->next;
				delete ptr;
			}
		}

		// Validate를 따로하지않음 의미가없음... 내가 락을 걸어놨으니까 Validate하는 건데, 락을 걸지 않는 락프리 자료구조에서는 의미가없음.
		//bool Validate(const Node* const pred, const Node* const curr)
		//{
		//	return !pred->marked && !curr->marked && pred->next == curr;
		//}
		void Find(_KeyType key, Node* (&pred), Node* (&curr))
		{
		retry:
			while (true)
			{
				pred = &head;
				curr = pred->next.GetPtr();

				while (true)
				{
					bool removed{};
					auto succ = curr->next.GetPtrWithRemoved(removed);

					while (removed)
					{
						if (!(pred->next.CAS(curr, succ, false, false))) { goto retry; }

						curr = succ;
						succ = curr->next.GetPtrWithRemoved(removed);
					}

					if (curr->key >= key) return;

					pred = curr;
					curr = succ;
				}
			}
		}

		bool Add(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			while (7)
			{
				Find(key, pred, curr);

				if (curr->key == key) { return false; }
				else
				{
					Node* addedNode = new Node(key);
					addedNode->next.Set(curr, false);

					if (pred->next.CAS(curr, addedNode, false, false)) { return true; }
				}
			}
		}

		bool Remove(const _KeyType key)
		{
			Node* pred{ nullptr };
			Node* curr{ nullptr };

			while (7)
			{
				Find(key, pred, curr);

				if (curr->key != key) { return false; }
				else
				{
					Node* deletedNode = curr->next.GetPtr();
					bool removed = curr->next.TryMark(deletedNode, true);

					if (!removed) continue;
					pred->next.CAS(curr, deletedNode, false, false);
					return true;
				}
			}
		}

		bool Contains(const _KeyType key)
		{
			Node* curr = &head;
			bool removed{};

			while (curr->key < key) { 
				curr = curr->next.GetPtr(); 
				Node* succ = curr->next.GetPtrWithRemoved(removed);
			}
			return curr->key == key && !removed;
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

int main() 
{
	using namespace LIST_4_LOCKFREE;

	for (int i = 1; i <= 8; i = i * 2)
	{
		std::vector<std::thread> threadCont;
		List list;
		threadCont.reserve(i);

 		auto startTime = high_resolution_clock::now();

		for (int j = 0; j < i; ++j)
		{
			threadCont.emplace_back([&]() {
				for (int k = 0, size = (GLOBAL::NUM_TEST / i)
					; k < size
					; k++)
				{
					switch (const int key = rand() % GLOBAL::KEY_RANGE; rand() % 3)
					{
					case 0: list.Add(key);	break;
					case 1: list.Remove(key); break;
					case 2: list.Contains(key); break;
					default: cout << "Error\n"; exit(-1);
					}
				}
				});
		}

		for (auto& thread : threadCont) { thread.join(); }

		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << i << "개의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

		CUSTOM_LIST::Display<LIST_4_LOCKFREE::List, LIST_4_LOCKFREE::Node>(list);
	}

	std::system("PAUSE");
	return 0;
}