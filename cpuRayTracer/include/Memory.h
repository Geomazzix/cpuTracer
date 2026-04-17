#pragma once

namespace CRT
{
    /**
     * @brief Allocates memory in alignment with the cache line size.
     * @param sizeInBytes The size in bytes to be allocated.
     * @return A valid memory address pointing to the allocated chunk of memory.
     */
    void* AllocAligned(size_t sizeInBytes);

    /**
     * @brief Must be called on memory that was allocated using AllocAligned, as opposed to delete or free.
     * @param address The address of the allocated chunk of memory.
     */
    void FreeAligned(void* address);

    /**
     * @brief Allocates an aligned amount of memory proportionate to the object type being provided.
     * @tparam T The type of object being provided.
     * @param count The number of objects being allocated.
     * @return A valid memory address pointing to the allocated chunk of memory.
     * @note Requires FreeAligned for cleanup.
     */
    template<typename T>
    T* AllocAligned(size_t count)
    {
        return static_cast<T*>(AllocAligned(count * sizeof(T)));
    }
}