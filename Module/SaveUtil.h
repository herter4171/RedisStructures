/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SaveUtil.h
 * Author: jwhh9
 *
 * Created on June 30, 2018, 7:48 PM
 */

#ifndef SAVEUTIL_H
#define SAVEUTIL_H

#include <array>

#include "../redismodule.h"

namespace SaveUtil
{
    template<typename T> // Shouldn't be called
    void save(RedisModuleIO *rdb, T &val)
    { 
        throw RedisException("Err saving not implemented for type!"); 
    }
    
    template<typename I> // Specialize for iterators
    void save(RedisModuleIO *rdb, I begin, I end)
    {
        for (I curr = begin; curr != end; ++curr)
            save(rdb, *curr);
    }
    
    template<typename T, std::size_t L> // Specialize for arrays
    void save(RedisModuleIO *rdb, std::array<T, L> &ary)
    {
        save(rdb, ary.begin(), ary.end());
    }
    
    /*template<typename T, typename... Args> // Specialize for a sequence of saves
    void save(RedisModuleIO *rdb, T &valT, Args... args)
    {
        save(rdb, valT);
        save(rdb, args... );
    }*/

    template<> // Specialize for doubles
    void save(RedisModuleIO *rdb, double &val){ RedisModule_SaveDouble(rdb, val); }
    
    template<> // Specialize for floats
    void save(RedisModuleIO *rdb, float &val){ RedisModule_SaveFloat(rdb, val); }

    template<> // Specialize for ints
    void save(RedisModuleIO *rdb, int64_t &val) { RedisModule_SaveSigned(rdb, val); }


}

#endif /* SAVEUTIL_H */

