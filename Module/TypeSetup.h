/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TypeSetup.h
 * Author: jwhh9
 *
 * Created on July 1, 2018, 4:11 AM
 */

#ifndef TYPESETUP_H
#define TYPESETUP_H

#include <string>

#include "../redismodule.h"
#include "../RedisException.h"

#include "LoadUtil.h"
#include "SaveUtil.h"

namespace TypeSetup
{
    template<typename T>
    RedisModuleTypeMethods setMethods(RedisModuleCtx *ctx, const char *name)
    {
        if (RedisModule_Init(ctx, name, 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
            throw RedisException("Err couldn't init module");
            
        RedisModuleTypeMethods tm;
        tm.version = REDISMODULE_TYPE_METHOD_VERSION;
        
        tm.rdb_load = [](RedisModuleIO *rdb, int encver)
        {
            T *refT = new T();            
            LoadUtil::load(rdb, refT);
            
            return (void*) refT;
        };
        
        tm.rdb_save = [](RedisModuleIO *rdb, void *value)
        {
            T *refT = (T*) value;
            SaveUtil::save(rdb, value);
        };
        
        tm.aof_rewrite = [](RedisModuleIO *aof, RedisModuleString *key, void *value) { }; // Not impl
        
        tm.free = [](void *value)
        {
            T* refT = (T*) value;
            delete refT;
        };
    }
    
    void setCommand(RedisModuleCtx *ctx, const RedisModuleCmdFunc func, std::string syntax)
    {
        if (RedisModule_CreateCommand(ctx, syntax.c_str(),
            func, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
        {
            std::string pfx = "Err creating command ";            
            throw RedisException(pfx + syntax);
        }
    }
}

#endif /* TYPESETUP_H */

