/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FunctionTypes.h
 * Author: jwhh9
 *
 * Created on June 30, 2018, 12:31 PM
 */

#ifndef FUNCTIONTYPES_H
#define FUNCTIONTYPES_H

#include <array>

#include "../constants.h"
#include "../redismodule.h"
#include "../RedisException.h"

namespace ParseUtil
{   
    
    template<typename T> // Interface for specialized parsing functions
    int parse(RedisModuleString *str, T &val)
    {
        if (parse(str, val) != REDISMODULE_OK)
            throw RedisException("Err couldn't parse value.");
    }
    
    template<> // Specialize for double
    int parse(RedisModuleString *str, double &val) { return RedisModule_StringToDouble(str, &val); }
    
    template<> // Specialize for integer
    int parse(RedisModuleString *str, long long &val) { return RedisModule_StringToLongLong(str, &val); }    
        
    
    template<typename I> // Specialize for iterator container
    void parse(I begin, I end, RedisModuleString **argv, std::size_t ArgOffset = ARG_COUNT_MIN)
    {
        std::size_t ind = ArgOffset;
        
        for (I curr = begin; curr != end; ++curr)
        {
            parse(argv[ind], *curr);
            ind++;
        }
    }
    
    template<typename T, std::size_t L> // TODO: Set back to iter
    void parse(std::array<T, L> &ary, RedisModuleString **argv, std::size_t ArgOffset)
    {
        for (int i = 0; i < L; i++)
        {
            double val;
            parse(argv[i + ArgOffset], val);
            ary[i] = val;
        }
    }
    

    

};



#endif /* FUNCTIONTYPES_H */

