#include "jobsystem.hpp"

namespace crt
{
	JobSystem::JobSystem() :
		m_jobPool(std::make_unique<ThreadSafeRingBuffer<ThreadJob, 256>>()),
		m_currentLabel(0)
	{
		m_finishedValue.store(0);

		uint32_t numWorkers = std::thread::hardware_concurrency();
		if (numWorkers == 0)
		{
			return;
		}
		numWorkers -= 1; /* -1 for the progress reporter. */

		m_workThreads.reserve(numWorkers);
		for (uint32_t i = 0; i < numWorkers; ++i)
		{
			m_workThreads.emplace_back([this](std::stop_token st) { WorkerThread(st); });
		}
	}

	JobSystem::~JobSystem()
	{
		for (auto& workerThread : m_workThreads)
		{
			workerThread.request_stop();
		}
	}

	void JobSystem::Execute(ThreadJob job)
	{
		m_currentLabel++;
		while (!m_jobPool->PushBack(job))
		{
			Poll();
		}

		m_wakeCondition.notify_one();
	}

	[[nodiscard]] bool JobSystem::IsBusy() const
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
		std::this_thread::yield();
	}

	void JobSystem::WorkerThread(std::stop_token stopToken)
	{
		ThreadJob job;
		while (!stopToken.stop_requested())
		{
			if (m_jobPool->PopFront(job))
			{
				job();
				m_finishedValue.fetch_add(1);
				continue;
			}

			std::unique_lock<std::mutex> lock(m_lockMutex);
			m_wakeCondition.wait(lock, stopToken, [&]
			{
				return !m_jobPool->IsEmpty();
			});
		}
	}
}