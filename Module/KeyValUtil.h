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
#include <string>

#include "../RedisException.h"
#include "../redismodule.h"
#include "../constants.h"

namespace KeyValUtil
{
    template<typename T>
    using UserKey_CB = std::function<void(RedisModuleCtx*, RedisModuleString**, int, T*)>;    
    
        
        
    template<int ReqArgCt>
    RedisModuleKey* fetchKey(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        // Check argument count
        if (ReqArgCt != argc){ throw RedisException(argc_err_msg); }

        // Get key ptr
        return (RedisModuleKey*) RedisModule_OpenKey(ctx, argv[ARG_KEY_IND], REDISMODULE_READ | REDISMODULE_WRITE);
    }
    
    template<int ReqArgCt = ARG_COUNT_MIN>
    int deleteKey(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisModuleType* redisType)
    {
        try
        {
            RedisModuleKey* key = fetchKey<ReqArgCt>(ctx, argv, argc);
            const char *msg = nullptr;
            
            if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY)
                throw RedisException("Err key already empty");
            
            if (RedisModule_ModuleTypeGetType(key) != redisType)
                throw RedisException("Err key is of wrong module type");
            
            if (RedisModule_DeleteKey(key) != REDISMODULE_OK)
                throw RedisException("Err key not open for writing");
            
            RedisModule_ReplyWithNull(ctx);            
        }
        catch(RedisException ex)
        {
            RedisModule_ReplyWithSimpleString(ctx, ex.what());
        }
        
        return REDISMODULE_OK;
    }   

    template<typename T, int ReqArgCt>
    T* fetchValue(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisModuleType *redisType)
    {
        RedisModuleKey *key = fetchKey<ReqArgCt>(ctx, argv, argc);
        
        T* refT;

        if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY)
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
        
        RedisModule_CloseKey(key); // Docs says safe to call on null
        
        return refT;
    }
    
    template<typename T, int ReqArgCt>
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
}


#endif /* KEYVALUTIL_H */

