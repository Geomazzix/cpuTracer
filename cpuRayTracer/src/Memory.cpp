#include "Memory.h"
#include <new>
#include <cstddef>

namespace CRT
{
    constexpr inline std::align_val_t GetL1CacheAlignment()
    {
        return static_cast<std::align_val_t>(std::hardware_destructive_interference_size);
    }

    void* AllocAligned(size_t sizeInBytes)
    {
        return ::operator new(sizeInBytes, GetL1CacheAlignment());
    }

    void FreeAligned(void* address)
    {
        ::operator delete(address, GetL1CacheAlignment());
    }
}