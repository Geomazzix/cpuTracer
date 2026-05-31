#pragma once
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "containers/threadsafeRingbuffer.hpp"

namespace crt
{
	using ThreadJob = std::function<void()>;
	
	/**
	 * @brief The job system manages multiple rays at the same time in order to improve the render time.- https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/
	 */
	class JobSystem final
	{
	public:
		JobSystem();
		~JobSystem();

		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;
		JobSystem(JobSystem&&) = delete;
		JobSystem& operator=(JobSystem&&) = delete;

		void Execute(ThreadJob job);

		[[nodiscard]] bool IsBusy() const;
		void Wait();

	private:
		std::unique_ptr<ThreadSafeRingBuffer<ThreadJob, 256>> m_jobPool;
		std::condition_variable_any m_wakeCondition;
		std::mutex m_lockMutex;
		std::atomic<uint64_t> m_finishedValue;
		std::atomic<uint64_t> m_currentLabel;
		std::vector<std::jthread> m_workThreads;

		void Poll();
		void WorkerThread(std::stop_token stopToken);
	};
}