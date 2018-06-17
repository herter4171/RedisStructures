/*
SOURCE: http://en.cppreference.com/w/cpp/named_req/Allocator
 */

#ifndef REDISALLOC_H
#define REDISALLOC_H

#include <cstdlib>
#include <new>

#include "redismodule.h"

template <class T>
struct RedisAlloc
{
    typedef T value_type;
    
    // Required for boost allocator.  Copied from new_allocator.hpp
    typedef T *                                  pointer;
    typedef const T *                            const_pointer;
    typedef T &                                  reference;
    typedef const T &                            const_reference;
    typedef std::size_t                          size_type;
    typedef std::ptrdiff_t                       difference_type;

    RedisAlloc() = default;

    template <class U> constexpr RedisAlloc(const RedisAlloc<U>&) noexcept
    { }

    T* allocate(std::size_t n)
    {
        if (n > std::size_t(-1) / sizeof (T))
            throw std::bad_alloc();

        //if (auto p = static_cast<T*> (std::malloc(n * sizeof (T))))
        if (auto p = static_cast<T*> (RedisModule_Alloc(n * sizeof (T))))
            return p;

        throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t) noexcept
    {
        //std::free(p); 
        RedisModule_Free(p);
    }
    
    // Required for boost allocator
    template<class U>
    struct rebind
    {
        typedef RedisAlloc<U> other;
    };
};

template <class T, class U>
bool operator==(const RedisAlloc<T>&, const RedisAlloc<U>&)
{
    return true;
}

template <class T, class U>
bool operator!=(const RedisAlloc<T>&, const RedisAlloc<U>&)
{
    return false;
}

#endif 