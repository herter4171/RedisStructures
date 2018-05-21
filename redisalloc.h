#ifndef REDISALLOC_H
#define REDISALLOC_H

// Source: https://stackoverflow.com/questions/21081796/why-not-to-inherit-from-stdallocator

#include <algorithm>
#include <cstdlib>
#include <forward_list>
#include <limits>

#include "redismodule.h"
template <typename T>
struct RedisAlloc
{
    using value_type = T;
    RedisAlloc() = default;

    template <class U>
    RedisAlloc(const RedisAlloc<U>&)
    { }

    T* allocate(std::size_t n)
    {
        T* ptrT;
        if (n <= std::numeric_limits<std::size_t>::max() / sizeof (T))
        {
            ptrT = (T*) RedisModule_Alloc(n*sizeof(T));
        }
        
        return ptrT;
    }

    void deallocate(T* ptr, std::size_t n)
    {
        
        for (int i = 0; i < n; i++)
        {
            RedisModule_Free(ptr + i*sizeof(T));
        }
    }
};

template <typename T, typename U>
inline bool operator==(const RedisAlloc<T>&, const RedisAlloc<U>&)
{
    return true;
}

template <typename T, typename U>
inline bool operator!=(const RedisAlloc<T>& a, const RedisAlloc<U>& b)
{
    return !(a == b);
}

#endif