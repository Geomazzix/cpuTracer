#include "JobSystem.h"

#pragma warning (push)
#pragma warning (disable : 26451)

namespace CRT
{
	JobSystem::JobSystem() :
		m_currentLabel(0)
	{}

	void JobSystem::Initialize()
	{
		m_finishedValue.store(0);
		uint32_t numThreads = std::max(1u, std::thread::hardware_concurrency() - 1); //-1 for the progress reporter.

		for (uint32_t i = 0; i < numThreads; ++i)
		{
			std::thread worker(&JobSystem::WorkerThread, this);
			worker.detach();
		}
	}

	void JobSystem::Execute(const ThreadJob& job)
	{
		m_currentLabel++;

		//Keep checking if a job can be added, if not keep notifying threads that there are unfinished jobs, 
		//in order to prevent them from falling asleep.
		while (!m_jobPool.PushBack(job))
		{
			Poll();
		}

		m_wakeCondition.notify_one();
	}

	bool JobSystem::IsBusy()
	{
		return m_finishedValue.load() < m_currentLabel;
	}

	void JobSystem::Wait()
	{
		while (IsBusy())
		{
			Poll();
		}
	}

	void JobSystem::Poll()
	{
		m_wakeCondition.notify_one();
		std::this_thread::yield();		//Allow the the thread to be rescheduled.
	}

	void JobSystem::WorkerThread()
	{
		ThreadJob job;

		while (true)
		{
			if (m_jobPool.PopFront(job))
			{
				job();
				m_finishedValue.fetch_add(1);
			}
			else
			{
				std::unique_lock<std::mutex> lock(m_lockMutex);
				m_wakeCondition.wait(lock);
			}
		}
	}
}

#pragma warning (pop)