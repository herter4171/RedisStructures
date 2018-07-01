/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   KeyParse.h
 * Author: jwhh9
 *
 * Created on June 26, 2018, 7:15 AM
 */

#ifndef KEYVALUTIL_H
#define KEYVALUTIL_H

#include <functional>
#include "../redismodule.h"
#include "../constants.h"

namespace KeyValUtil
{
    template<typename T>
    using UserKey_CB = std::function<void(RedisModuleCtx*, RedisModuleString**, int, T*)>;    
    
        
    template<std::size_t ReqArgCt>//, int OpenFlags>
    RedisModuleKey* fetchKey(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        // Check argument count
        if (argc != ReqArgCt)
            throw RedisException("ERR wrong number of arguments!");

        // Get key ptr
        return (RedisModuleKey*) RedisModule_OpenKey(ctx, argv[ARG_KEY_IND], REDISMODULE_READ | REDISMODULE_WRITE);
    }   
    
    /*template<std::size_t ReqArgCt>
    RedisModuleKey* fetchKey(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        return fetchKey<ReqArgCt, REDISMODULE_READ | REDISMODULE_WRITE>(ctx, argv, argc);
    }*/
    
    /*template<>
    RedisModuleKey* fetchKey(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        return fetchKey<ARG_COUNT_MIN, REDISMODULE_READ>(ctx, argv, argc);
    }*/
    
    template<typename T, std::size_t ReqArgCt>
    T* fetchValue(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisModuleType *redisType)
    {
        RedisModuleKey *key = fetchKey<ReqArgCt>(ctx, argv, argc);
        int type = RedisModule_KeyType(key);            
        
        T* refT;

        if (type == REDISMODULE_KEYTYPE_EMPTY)
        {
            refT = new T();
            RedisModule_ModuleTypeSetValue(key, redisType, refT);
        }
        else if (RedisModule_ModuleTypeGetType(key) == redisType)
        {
            refT = (T*) RedisModule_ModuleTypeGetValue(key);
        }
        else
        {
            throw RedisException(REDISMODULE_ERRORMSG_WRONGTYPE);
        }
        
        return refT;
    }
    
    template<typename T, std::size_t ReqArgCt>
    int runCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisModuleType* redisType,  UserKey_CB<T> func)
    {
        try
        {
            T *refT = fetchValue<T, ReqArgCt>(ctx, argv, argc, redisType);
            func(ctx, argv, argc, refT);
        } 
        catch (RedisException exc)
        {
            return RedisModule_ReplyWithError(ctx, exc.what());
        }

        return REDISMODULE_OK;
    }
    
    template<typename T, std::size_t ReqArgCt>
    std::function<int(RedisModuleCtx*, RedisModuleString**, int)> getCommandFunction(UserKey_CB<T> func, RedisModuleType *modType)
    {
        std::function<int(RedisModuleCtx*, RedisModuleString**, int)> redis_func = 
        [func, modType](RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
        {
            return runCommand<T, ReqArgCt>(ctx, argv, argc, modType, func);
        };
        
        return redis_func;
    }
    
    
    
    
    
        /*template<typename T, std::size_t ReqArgCt>
        RedisModuleCmdFunc makeCommand(UserKey_CB<T> func, RedisModuleType *modType)
        {
            
            RedisModuleCmdFunc redis_func = [&](RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
            {
                T *refT = fetchValue<T, ReqArgCt>(ctx, argv, argc, modType);
                //func(ctx, argv, argc, refT);
                return REDISMODULE_OK;
            };
            
            return redis_func;
        }*/
}


#endif /* KEYVALUTIL_H */

