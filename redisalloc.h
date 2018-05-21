/*
The following code example is taken from the book
The C++ Standard Library - A Tutorial and Reference
by Nicolai M. Josuttis, Addison-Wesley, 1999
� Copyright Nicolai M. Josuttis 1999
 */

#ifndef REDISALLOC_H
#define REDISALLOC_H

#include <limits>
#include <iostream>
#include "redismodule.h"

template <class T>
class RedisAlloc {
public:
    // type definitions
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U

    template <class U>
    struct rebind {
        typedef RedisAlloc<U> other;
    };

    // return address of values

    pointer address(reference value) const {
        return &value;
    }

    const_pointer address(const_reference value) const {
        return &value;
    }

    /* constructors and destructor
     * - nothing to do because the allocator has no state
     */
    RedisAlloc() throw () {
    }

    RedisAlloc(const RedisAlloc&) throw () {
    }

    template <class U>
    RedisAlloc(const RedisAlloc<U>&) throw () {
    }

    ~RedisAlloc() throw () {
    }

    // return maximum number of elements that can be allocated

    size_type max_size() const throw () {
        return std::numeric_limits<std::size_t>::max() / sizeof (T);
    }

    // allocate but don't initialize num elements of type T

    pointer allocate(size_type num, const void* = 0) {
        pointer ret = (pointer) (RedisModule_Alloc(num * sizeof (T)));
        return ret;
    }

    // initialize elements of allocated storage p with value value

    void construct(pointer p, const T& value) {
        p = (pointer) RedisModule_Alloc(sizeof (value));
        *p = value;
    }

    // destroy elements of initialized storage p

    void destroy(pointer p) {
        RedisModule_Free((void*) p);
    }

    // deallocate storage p of deleted elements

    void deallocate(pointer p, size_type num) {
        for (int i = 0; i < num; i++)
            destroy(p + sizeof (T) * i);
    }
};

// return that all specializations of this allocator are interchangeable

template <class T1, class T2>
bool operator==(const RedisAlloc<T1>&,
        const RedisAlloc<T2>&) throw () {
    return true;
}

template <class T1, class T2>
bool operator!=(const RedisAlloc<T1>&,
        const RedisAlloc<T2>&) throw () {
    return false;
}

#endif // !REDISALLOC_H
