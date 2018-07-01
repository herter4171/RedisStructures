/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LoadUtil.h
 * Author: jwhh9
 *
 * Created on June 30, 2018, 8:16 PM
 */

#ifndef LOADUTIL_H
#define LOADUTIL_H

#include <array>
#include "../redismodule.h"

namespace LoadUtil
{
    template<typename T> // Shouldn't be called
    void load(RedisModuleIO *rdb, T &val)
    { 
        throw RedisException("Err saving not implemented for type!"); 
    }
    
    template<typename I> // Specialize for iterator container
    void load(RedisModuleIO *rdb, I begin, I end)
    {
        for (I curr = begin; curr != end; ++curr)
            load(rdb, *curr);
    }
    
    template<typename T, std::size_t L> // Specialize for arrays
    void load(RedisModuleIO *rdb, std::array<T, L> &ary)
    {
        load(rdb, ary.begin(), ary.end());
    }

    template<> // Specialize for doubles
    void load(RedisModuleIO *rdb, double &val){ val = RedisModule_LoadDouble(rdb); }
    
    template<> // Specialize for floats
    void load(RedisModuleIO *rdb, float &val){ val = RedisModule_LoadFloat(rdb); }

    template<> // Specialize for ints
    void load(RedisModuleIO *rdb, int64_t &val) { val = RedisModule_LoadSigned(rdb); }

}

#endif /* LOADUTIL_H */

