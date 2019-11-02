// cpp
#include <iostream>
#include <chrono>

// C++11
#include <mutex>
#include <thread>
#include <atomic>

// C++ STL
#include <vector>

#include "CustomQueue.h"

using namespace std;
using namespace std::chrono;

namespace QUEUE_0_COARSE_GRAINED_SYNC {
	
	// -----------------
	// Node
	// -----------------
	Node::Node() noexcept
		: key()
		, next(nullptr)
	{
	}

	Node::Node(const _KeyType keyValue)
		: key(keyValue)
		, next(nullptr)
	{
	}

	// -----------------
	// Queue
	// -----------------
	Queue::Queue() noexcept
		: head(new Node(0))
		, tail(head)
	{
		head->next = nullptr;
	}

	Queue::~Queue()
	{
		Init();
	}

	void Queue::Init()
	{
		Node* ptr;

		while (head->next != nullptr)
		{
		 	ptr = head->next;
		 	head->next = head->next->next;
		 	delete ptr;
		}
		 
		delete head;
		delete tail;
	}

	void Queue::Display(const int inCount)
	{
		Node* ptr = head->next;
		int count = inCount;

		while (ptr != nullptr)
		{
			std::cout << inCount - count << ": " << ptr->key << " , ";
			ptr = ptr->next;

			if (--count <= 0) break;
		}
	}

	void Queue::Enq(const _KeyType key)
	{
		{
			std::unique_lock<std::mutex> LocalLock(queueLock);
			Node* pEnqNode = new Node(key);
			tail->next = pEnqNode;
			tail = pEnqNode;
		}
	}

	Node* Queue::Deq()
	{
		Node* pDeqNode{ nullptr };
		{
			std::unique_lock<std::mutex> LocalLock(queueLock);

			if (head->next == nullptr)
			{
				return nullptr;
			}
			pDeqNode = head->next;
			head->next = head->next->next;
		}
		return pDeqNode;
	}
}

int main()
{
	using namespace QUEUE_0_COARSE_GRAINED_SYNC;

	for (int i = 1; i <= 8; i = i * 2)
	{
		Queue queue;
		std::cout <<" hi \n";

		std::vector<std::thread> threadCont;
		threadCont.reserve(i);

		auto startTime = high_resolution_clock::now();

		for (int j = 0; j < 1000; ++j)
		{
			queue.Enq(0);
		}

		for (int j = 0; j < i; ++j)
		{
			threadCont.emplace_back([&]() {
				for (int k = 0, size = (GLOBAL::NUM_TEST / i)
					; k < size
					; k++)
				{
					switch (const int key = rand() % GLOBAL::KEY_RANGE
						; rand() % 2)
					{
					case 0: queue.Enq(j);	break;
					case 1: queue.Enq(j);	break;

						//					case 1: Node* ptr = queue.Deq();  delete ptr;	break;
											//default: cout << "Error\n"; exit(-1);
					}
				}
			});
		}

		for (auto& thread : threadCont) { thread.join(); }

		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << i << "개의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

		queue.Display();
	}

	std::system("PAUSE");
	return 0;
}
