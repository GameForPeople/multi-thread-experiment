#include "stdafx.h"

#include "Shared_Mutex_WithGuard.h"

/*
	shared_lock�� RAII�� ���ϴ�.
		��?
			- �Ǽ� ����
			- ���� �� release���� ������ �ٸ����� Jump�ϴµ� �̷��� Lock ��ǰ. �̷� ��� ����.
			- ���� ���� X ���������.

	 ����KeyWord
		std::lock_guard<T>  - ��� mutex �����ų� �־ ��� lock�Ҷ� ��.
		std::unique_lock<T> - ���� ����ȣȯ. wrLock���� wLock�� �̰ɷ� ����
		std::shared_lock<T> - wrLock�� rLock�� �̰ɷ� ����. ����.

		���� �����ؼ� ��ȭ������ ���� �����ؼ� �� �غ�����.
*/
void Shared_Mutex_WithGuard::TestFunc()
{
	std::shared_mutex rootLock;
	std::vector<std::thread> writeThreadCont;
	std::vector<std::thread> readThreadCont;
	
	int globalSum{ 0 };
	const int loopSize { 10000000 };

	writeThreadCont.resize(2);
	readThreadCont.resize(4);

	for (auto& thread : writeThreadCont)
	{
		thread = std::thread
		([&] 
			{
				for (int i = 0; i < loopSize; ++i)
				{
					std::unique_lock<std::shared_mutex> localWriteLock(rootLock);
					++globalSum;
				}
			}
		);
	}

	for (auto& thread : readThreadCont)
	{
		thread = std::thread
		([&]
			{
				int tempLocal{};

				for (int i = 0; i < 100; ++i)
				{
					{
						std::shared_lock<std::shared_mutex> localWriteLock(rootLock);
						tempLocal = globalSum;
					}
					this_thread::sleep_for(std::chrono::milliseconds(50));
				}

				this_thread::sleep_for(std::chrono::seconds(1));

				{
					std::unique_lock<std::shared_mutex> localWriteLock(rootLock);
					std::cout << this_thread::get_id() << " : " << tempLocal << "\n";
				}
			}
		);
	}

	for (auto& thread : writeThreadCont) thread.join();
	for (auto& thread : readThreadCont) thread.join();
}