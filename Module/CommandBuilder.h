/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Command.h
 * Author: jwhh9
 *
 * Created on June 21, 2018, 6:05 PM
 */

#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include "../redismodule.h"

class CommandBuilder
{
public:
    CommandBuilder(std::string syntax_, bool readonly, const RedisModuleCmdFunc func_):func(func_)
    {
        syntax = syntax_;
        
        if (readonly)
            flags = "readonly";
        else
            flags = "write deny-oom";
    }
    
    void set(RedisModuleCtx *ctx)
    {
        if (RedisModule_CreateCommand(ctx, syntax.c_str(),
            func, flags.c_str(), 1, 1, 1) == REDISMODULE_ERR)
        {
            std::string pfx = "Err creating command ";            
            throw RedisException(pfx + syntax);
        }
    }
    
    
    
private:
    std::string syntax, flags;
    const RedisModuleCmdFunc func;
};

#endif /* COMMAND_H */

