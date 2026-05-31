#pragma once
#include <cstdint>
#include <mutex>
#include <array>

namespace crt
{
	/**
	 * @brief Thread-safe ring buffer.
	 * @tparam T The type of data to store in the buffer.
	 * @tparam Capacity The amount of items allowed in the ring buffer.
	 */
	template<typename T, size_t Capacity>
	class ThreadSafeRingBuffer final
	{
	public:
		ThreadSafeRingBuffer();
		~ThreadSafeRingBuffer() = default;

		ThreadSafeRingBuffer(const ThreadSafeRingBuffer&) = delete;
		ThreadSafeRingBuffer& operator=(const ThreadSafeRingBuffer&) = delete;
		ThreadSafeRingBuffer(ThreadSafeRingBuffer&&) = delete;
		ThreadSafeRingBuffer& operator=(ThreadSafeRingBuffer&&) = delete;

		template<typename U> requires std::is_constructible_v<T, U&&>
		bool PushBack(U&& item);
		bool PopFront(T& item);

		[[nodiscard]] bool IsEmpty() const;
		[[nodiscard]] size_t GetCapacity() const;

	private:
		std::array<T, Capacity> m_data;
		mutable std::mutex m_ringBufferMutex;
		size_t m_head;
		size_t m_tail;
	};


	template<typename T, size_t Capacity>
	ThreadSafeRingBuffer<T, Capacity>::ThreadSafeRingBuffer() :
		m_head{ 0 },
		m_tail{ 0 }
	{}

	template<typename T, size_t Capacity>
	template<typename U> requires std::is_constructible_v<T, U&&>
	inline bool ThreadSafeRingBuffer<T, Capacity>::PushBack(U&& item)
	{	
		std::scoped_lock lock(m_ringBufferMutex);

		const size_t next = (m_head + 1) % Capacity;
		if (next == m_tail)
		{
			return false;
		}
		
		m_data[m_head] = std::forward<U>(item);
		m_head = next;
		return true;
	}

	template<typename T, size_t Capacity>
	inline bool ThreadSafeRingBuffer<T, Capacity>::PopFront(T& item)
	{
		std::scoped_lock lock(m_ringBufferMutex);
		if (m_tail == m_head) 
		{
			return false;
		}

		item = std::move(m_data[m_tail]);
		m_tail = (m_tail + 1) % Capacity;
		return true;
	}

	template<typename T, size_t Capacity>
	inline bool ThreadSafeRingBuffer<T, Capacity>::IsEmpty() const
	{
		std::scoped_lock lock(m_ringBufferMutex);
		return m_head == m_tail;
	}

	template<typename T, size_t Capacity>
	inline size_t ThreadSafeRingBuffer<T, Capacity>::GetCapacity() const
	{
		return Capacity;
	}
}