#include <concepts>
#include <cstddef>
#include <vector>

namespace crt
{
	/**
	 * @brief Minimum required feature-set for a SlotMapKey custom implementation.
	 */
	template<typename K>
	concept SlotMapKeyType = requires
	{
		typename K::size_type;
	}
	&& requires(K k, typename K::size_type idx, typename K::size_type gen)
	{
		{ K(idx, gen) } -> std::same_as<K>;
		{ k.GetIndex() } -> std::same_as<typename K::size_type>;
		{ k.GetGeneration() } -> std::same_as<typename K::size_type>;
		{ k.IsValid() } -> std::same_as<bool>;
		{ K::InvalidHandle() } -> std::same_as<K>;
	}
	&& std::is_default_constructible_v<K>
		&& std::is_copy_constructible_v<K>&& std::is_copy_assignable_v<K>
		&& std::is_move_constructible_v<K>&& std::is_move_assignable_v<K>;

	/**
	 * @brief Constraints on the minimum requirements for the Slot-map value_type.
	 */
	template<typename T>
	concept SlotMapValueType = (std::is_default_constructible_v<T> || std::is_constructible_v<T, std::nullptr_t>)
		&& std::is_move_constructible_v<T>
		&& std::is_move_assignable_v<T>;

	/**
	 * @brief Default key type for the slot-map. Custom ones are allowed and in most scenarios recommended for better
	 * bit-precision ratios between the index and generation count.
	 * @tparam SizeType The unsigned data type used, i.e., u8/u16/u32/u64. Defaults to u32.
	 */
	export template<UnsignedIntegral SizeType = u32>
	struct SlotMapKey final
	{
		using size_type = SizeType;

		constexpr SlotMapKey();
		constexpr SlotMapKey(size_type index, size_type generation);

		[[nodiscard]] constexpr size_type GetIndex() const;
		[[nodiscard]] constexpr size_type GetGeneration() const;

		[[nodiscard]] constexpr bool IsValid() const;
		[[nodiscard]] static constexpr SlotMapKey InvalidHandle();

		[[nodiscard]] constexpr bool operator==(const SlotMapKey&) const = default;
		[[nodiscard]] constexpr auto operator<=>(const SlotMapKey&) const = default;

	private:
		size_type m_index;
		size_type m_generation;
	};

	template<UnsignedIntegral SizeType>
	constexpr SlotMapKey<SizeType>::SlotMapKey() :
		m_index(std::numeric_limits<SizeType>::max()),
		m_generation(std::numeric_limits<SizeType>::max())
	{}

	template<UnsignedIntegral SizeType>
	constexpr SlotMapKey<SizeType>::SlotMapKey(size_type index, size_type generation) :
		m_index(index),
		m_generation(generation)
	{}

	template<UnsignedIntegral SizeType>
	constexpr SizeType SlotMapKey<SizeType>::GetIndex() const
	{
		return m_index;
	}

	template<UnsignedIntegral SizeType>
	constexpr SizeType SlotMapKey<SizeType>::GetGeneration() const
	{
		return m_generation;
	}

	template<UnsignedIntegral SizeType>
	constexpr bool SlotMapKey<SizeType>::IsValid() const
	{
		return m_index != std::numeric_limits<SizeType>::max() && m_generation != std::numeric_limits<SizeType>::max();
	}

	template<UnsignedIntegral SizeType>
	constexpr SlotMapKey<SizeType> SlotMapKey<SizeType>::InvalidHandle()
	{
		return SlotMapKey{ std::numeric_limits<SizeType>::max(), std::numeric_limits<SizeType>::max() };
	}


	/**
	 * @brief A SlotMap is a contiguous array with generational index handles that provides O(1) insert, lookup,
	 * and erase while detecting use-after-free through generation counters.
	 * @note This container does not support iteration by design — it is strictly a handle-based lookup structure.
	 * @tparam Value The value type to store.
	 * @tparam Key The handle/key type must satisfy SlotMapKey.
	 * @tparam Allocator The allocator type used for internal storage.
	 */
	export template<SlotMapValueType Value, SlotMapKeyType Key = SlotMapKey<>, typename Allocator = std::allocator<Value>>
	class SlotMap final
	{
	public:
		using key_type = Key;
		using size_type = typename Key::size_type;
		using value_type = Value;
		using pointer = Value*;
		using const_pointer = const Value*;
		using allocator_type = Allocator;

		constexpr SlotMap();
		constexpr explicit SlotMap(usize initialCapacity);
		constexpr explicit SlotMap(const Allocator& alloc);
		constexpr ~SlotMap() = default;

		constexpr SlotMap(const SlotMap&) = default;
		constexpr SlotMap& operator=(const SlotMap&) = default;

		constexpr SlotMap(SlotMap&& other) noexcept = default;
		constexpr SlotMap& operator=(SlotMap&& other) noexcept = default;

		/**
		 * @brief Returns true if no live elements exist.
		 */
		[[nodiscard]] constexpr bool IsEmpty() const noexcept;

		/**
		 * @brief Returns the number of live elements.
		 */
		[[nodiscard]] constexpr usize GetOccupied() const noexcept;

		/**
		 * @brief Returns the total number of allocated slots.
		 */
		[[nodiscard]] constexpr usize GetSize() const noexcept;

		/**
		 * @brief Returns the number of elements that can be held in currently allocated storage.
		 */
		[[nodiscard]] constexpr usize GetCapacity() const noexcept;

		/**
		 * @brief Pre-allocates storage for at least newCapacity elements.
		 */
		constexpr void Reserve(usize newCapacity);

		/**
		 * @brief Looks up an element by handle.
		 * @param handle The handle returned from Insert/Emplace.
		 * @return Pointer to the element, or nullptr if the handle is stale or invalid.
		 */
		[[nodiscard]] constexpr pointer Find(key_type handle);

		/**
		 * @copydoc Find(key_type)
		 */
		[[nodiscard]] constexpr const_pointer Find(key_type handle) const;

		/**
		 * @brief Returns true if the handle refers to a live element.
		 * @param handle The handle to check.
		 */
		[[nodiscard]] constexpr bool Contains(key_type handle) const noexcept;

		/**
		 * @brief Inserts an element into the SlotMap.
		 * @param value The value to insert.
		 * @return A handle to the inserted element.
		 */
		template<typename U>
		constexpr key_type Insert(U&& value);

		/**
		 * @brief Constructs an element in-place inside the SlotMap.
		 * @tparam Args Constructor argument types.
		 * @param args  Arguments forwarded to Value's constructor.
		 * @return A handle to the constructed element.
		 */
		template<typename... Args>
		constexpr key_type Emplace(Args&&... args);

		/**
		 * @brief Erases the element referred to by the handle.
		 * @param handle The handle of the element to erase.
		 * @return True if an element was erased, false if the handle was stale or invalid.
		 */
		constexpr bool Erase(key_type handle);

		/**
		 * @brief Destroys all live elements and resets the SlotMap to an empty state.
		 *        All outstanding handles are invalidated. Allocated memory is retained.
		 */
		constexpr void Clear() noexcept;

		/**
		 * @brief Swaps the contents of two SlotMaps.
		 */
		constexpr void Swap(SlotMap& other) noexcept;

	private:
		[[nodiscard]] constexpr bool IsHandleValid(key_type handle) const noexcept;

		std::vector<Value, Allocator> m_data;
		std::vector<size_type> m_generations;
		std::vector<size_type> m_freeList;
		usize m_liveCount;
	};

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr SlotMap<Value, Key, Allocator>::SlotMap()
		: m_liveCount(0)
	{}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr SlotMap<Value, Key, Allocator>::SlotMap(usize initialCapacity)
		: m_liveCount(0)
	{
		Reserve(initialCapacity);
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr SlotMap<Value, Key, Allocator>::SlotMap(const Allocator& alloc)
		: m_data(alloc)
		, m_liveCount(0)
	{}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr bool SlotMap<Value, Key, Allocator>::IsEmpty() const noexcept
	{
		return m_liveCount == 0;
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr usize SlotMap<Value, Key, Allocator>::GetOccupied() const noexcept
	{
		return m_liveCount;
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr usize SlotMap<Value, Key, Allocator>::GetSize() const noexcept
	{
		return m_data.size();
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr usize SlotMap<Value, Key, Allocator>::GetCapacity() const noexcept
	{
		return m_data.capacity();
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr void SlotMap<Value, Key, Allocator>::Reserve(usize newCapacity)
	{
		m_data.reserve(newCapacity);
		m_generations.reserve(newCapacity);
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr SlotMap<Value, Key, Allocator>::pointer  SlotMap<Value, Key, Allocator>::Find(key_type handle)
	{
		return !IsHandleValid(handle) ? nullptr : &m_data[handle.GetIndex()];
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr SlotMap<Value, Key, Allocator>::const_pointer SlotMap<Value, Key, Allocator>::Find(key_type handle) const
	{
		return !IsHandleValid(handle) ? nullptr : &m_data[handle.GetIndex()];
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr bool SlotMap<Value, Key, Allocator>::Contains(key_type handle) const noexcept
	{
		return IsHandleValid(handle);
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	template<typename U>
	constexpr SlotMap<Value, Key, Allocator>::key_type SlotMap<Value, Key, Allocator>::Insert(U&& value)
	{
		size_type index;

		if (!m_freeList.empty())
		{
			index = m_freeList.back();
			m_freeList.pop_back();
			m_data[index] = std::forward<U>(value);
		}
		else
		{
			index = static_cast<size_type>(m_data.size());
			m_data.push_back(std::forward<U>(value));
			m_generations.push_back(0);
		}

		++m_liveCount;

		return key_type(index, m_generations[index]);
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	template<typename... Args>
	constexpr SlotMap<Value, Key, Allocator>::key_type SlotMap<Value, Key, Allocator>::Emplace(Args&&... args)
	{
		return Insert(Value(std::forward<Args>(args)...));
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr bool SlotMap<Value, Key, Allocator>::Erase(key_type handle)
	{
		if (!IsHandleValid(handle))
		{
			return false;
		}

		const auto idx = handle.GetIndex();

		if constexpr (std::is_default_constructible_v<Value>)
		{
			m_data[idx] = Value{};
		}
		else
		{
			m_data[idx] = Value{ nullptr };
		}

		++m_generations[idx];
		m_freeList.push_back(idx);
		--m_liveCount;

		return true;
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr void SlotMap<Value, Key, Allocator>::Clear() noexcept
	{
		for (usize i = 0; i < m_data.size(); ++i)
		{
			if constexpr (std::is_default_constructible_v<Value>)
			{
				m_data[i] = Value{};
			}
			else
			{
				m_data[i] = Value{ nullptr };
			}
		}

		m_freeList.clear();
		m_freeList.reserve(m_data.size());

		for (size_type i = 0; i < static_cast<size_type>(m_data.size()); ++i)
		{
			++m_generations[i];
			m_freeList.push_back(i);
		}

		m_liveCount = 0;
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr void SlotMap<Value, Key, Allocator>::Swap(SlotMap& other) noexcept
	{
		std::swap(m_data, other.m_data);
		std::swap(m_generations, other.m_generations);
		std::swap(m_freeList, other.m_freeList);
		std::swap(m_liveCount, other.m_liveCount);
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr bool SlotMap<Value, Key, Allocator>::IsHandleValid(key_type handle) const noexcept
	{
		if (!handle.IsValid())
		{
			return false;
		}

		const auto idx = handle.GetIndex();
		if (idx >= static_cast<size_type>(m_data.size()))
		{
			return false;
		}

		return m_generations[idx] == handle.GetGeneration();
	}

	template<SlotMapValueType Value, SlotMapKeyType Key, typename Allocator>
	constexpr void swap(SlotMap<Value, Key, Allocator>& lhs, SlotMap<Value, Key, Allocator>& rhs) noexcept
	{
		lhs.Swap(rhs);
	}
}
