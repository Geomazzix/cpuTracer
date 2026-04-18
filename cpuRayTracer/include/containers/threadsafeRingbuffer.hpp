#pragma once
#include <cstdint>
#include <mutex>

namespace CRT
{
	/**
	 * @brief Minimal thread safe ring buffer.
	 * @tparam T The type of data to store in the buffer.
	 * @tparam Capacity The amount of items allowed in the ring buffer.
	 */
	template<typename T, size_t Capacity>
	class ThreadSafeRingBuffer
	{
	public:
		ThreadSafeRingBuffer();
		~ThreadSafeRingBuffer() = default;

		bool PushBack(const T& item);
		bool PopFront(T& item);

		size_t GetCapacity() const;

	private:
		size_t m_head;
		size_t m_tail;
		T m_data[Capacity];
		std::mutex m_ringBufferMutex;
	};


	template<typename T, size_t Capacity>
	ThreadSafeRingBuffer<T, Capacity>::ThreadSafeRingBuffer() :
		m_head(0),
		m_tail(0)
	{}

	template<typename T, size_t Capacity>
	bool ThreadSafeRingBuffer<T, Capacity>::PopFront(T& item)
	{
		bool result = false;
		m_ringBufferMutex.lock();

		if (m_tail != m_head)
		{
			item = m_data[m_tail];
			m_tail = (m_tail + 1) % Capacity;
			result = true;
		}

		m_ringBufferMutex.unlock();
		return result;
	}

	template<typename T, size_t Capacity>
	bool ThreadSafeRingBuffer<T, Capacity>::PushBack(const T& item)
	{
		bool result = false;
		m_ringBufferMutex.lock();

		size_t next = (m_head + 1) % Capacity;
		if (next != m_tail)
		{
			m_data[m_head] = item;
			m_head = next;
			result = true;
		}

		m_ringBufferMutex.unlock();
		return result;
	}

	template<typename T, size_t Capacity>
	size_t ThreadSafeRingBuffer<T, Capacity>::GetCapacity() const
	{
		return Capacity;
	}
}