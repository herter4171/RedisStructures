/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModuleBuilder.h
 * Author: jwhh9
 *
 * Created on June 21, 2018, 9:01 PM
 */

#ifndef MODULEBUILDER_H
#define MODULEBUILDER_H

#include <string>
#include <vector>
#include <tuple>
#include <functional>

#include "../redismodule.h"
#include "../RedisException.h"

#include "CommandBuilder.h"
#include "../constants.h"

template<typename T>
class ModuleBuilder
{
public:
    
    
    ModuleBuilder(std::string pfx_, RedisModuleType *modType_)
    {
        pfx = pfx_;
        modType = modType_;
    }
    

    
    int build(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        REDISMODULE_NOT_USED(argv);
        REDISMODULE_NOT_USED(argc);
        
        int status = REDISMODULE_OK;
        
        try
        {
            RedisModuleTypeMethods tm = makeType(ctx);
            setCommands(ctx);   

            modType = RedisModule_CreateDataType(ctx, pfx.c_str(), 0, &tm);
            
            if (modType == NULL)
            {
                std::string msg = "Err couldn't create datatype";
                msg += pfx;
                
                throw RedisException(msg);
            }

        } 
        catch (RedisException ex)
        {
            RedisModule_ReplyWithError(ctx, ex.what());
        }

        return status;
    }
    
protected:
    typedef std::tuple<std::string, bool, std::function<int(RedisModuleCtx*, RedisModuleString**, int)>> cmd_tuple;
    
    RedisModuleType *modType;
    
    // Sets members for RedisModuleTypeMethods instance
    RedisModuleTypeMethods makeType(RedisModuleCtx *ctx)
    {
        RedisModuleTypeMethods tm;
        tm.rdb_load = getLoader();
        tm.rdb_save = getSaver();
        tm.aof_rewrite = getRewriter();
        tm.free = getFreer();
        
        return tm;
    }
    
    // Assigns functions for module commands
    void setCommands(RedisModuleCtx *ctx)
    {
        for (CommandBuilder &cmd : getCommandBuilders())
        {
            cmd.set(ctx);
        }
    }
    
    
    
    virtual std::vector<CommandBuilder> getCommandBuilders()=0;
    
    // Return lambdas for establishing a RedisModuleTypeMethods instance
    virtual RedisModuleTypeLoadFunc  getLoader()=0;    
    virtual RedisModuleTypeSaveFunc getSaver()=0;    
    virtual RedisModuleTypeRewriteFunc getRewriter()=0;    
    virtual RedisModuleTypeFreeFunc getFreer()=0;
    
    static T* getInstance(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisModuleType *pModType)
    {
        if (argc < ARG_COUNT_MIN)
            throw RedisException("ERR wrong number of arguments!");

        RedisModuleKey *key = (RedisModuleKey*) RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
        int type = RedisModule_KeyType(key);

        T *ptrT;
        if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != pModType)
            throw RedisException(REDISMODULE_ERRORMSG_WRONGTYPE);
        else if (type == REDISMODULE_KEYTYPE_EMPTY)
        {
            ptrT = new T(); // Using RedisAlloc<T> is a bad time here
            RedisModule_ModuleTypeSetValue(key, pModType, ptrT);
        } else
        {
            ptrT = (T*) RedisModule_ModuleTypeGetValue(key);
        }

        return ptrT;
    }   
    
private:
    std::string pfx;
    
    

    
};

#endif /* MODULEBUILDER_H */

