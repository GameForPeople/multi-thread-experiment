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
		: head(0)
		, tail(&head)
	{
	}

	Queue::~Queue()
	{
		Init();
	}

	void Queue::Init()
	{
		Node* ptr{ nullptr };

		while (head.next != tail)
		{
		 	ptr = head.next;
		 	head.next = ptr->next;
		 	delete ptr;
		}

		delete tail; // == head.next
	}

	void Queue::Display(int count /* copy */)
	{
		Node* ptr = head.next;

		while (ptr != nullptr)
		{
			std::cout << " " << ptr->key << ",";
			ptr = ptr->next;

			if (--count <= 0) break;
		}

		std::cout << "\n";
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

			if (head.next == nullptr) { return nullptr; }
			
			pDeqNode = head.next;
			head.next = head.next->next;
		}
		return pDeqNode;
	}
}

int main11()
{
	using namespace QUEUE_0_COARSE_GRAINED_SYNC;

	for (int i = 1; i <= 8; i = i * 2)
	{
		Queue queue;

		std::vector<std::thread> threadCont;
		threadCont.reserve(i);

		auto startTime = high_resolution_clock::now();
		{
			for (int j = 0; j < 100000; ++j) { queue.Enq(j); }

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
							case 0: 
							{
								queue.Enq(k);	
								break;
							}
							case 1: 
							{
								delete queue.Deq();
								break;
							}
							default: 
								cout << "Error\n"; exit(-1);
						}
					}
				});
			}

			for (auto& thread : threadCont) { thread.join(); }
		}
		auto endTime = high_resolution_clock::now() - startTime;
		std::cout << "\n 쓰레드 "<< i << "개의 성능은? " << duration_cast<milliseconds>(endTime).count() << " msecs\n";

		queue.Display();
	}

	std::system("PAUSE");
	return 0;
}
