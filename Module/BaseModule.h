/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BaseModuel.h
 * Author: jwhh9
 *
 * Created on July 1, 2018, 12:58 PM
 */

#ifndef BASEMODULE_H
#define BASEMODULE_H

#include <string>
#include <vector>
#include <map>

#include <boost/algorithm/string/join.hpp>

#include "../redismodule.h"
#include "TypeSetup.h"


template<typename DataType>
class BaseModule
{
    public:
        BaseModule(std::string name_): name(name_)
        { }
    
        void initialize(RedisModuleCtx *ctx)
        {
            RedisModuleTypeMethods tm = setMethods(ctx);
            
            for (auto &pair : getCommands())
            {
                std::string syntax = boost::algorithm::join(std::vector<std::string>({name, pair.first}), ".");
                TypeSetup::setCommand(ctx, pair.second, syntax);
            }
            
            modType = RedisModule_CreateDataType(ctx, name.c_str(), 0, &tm);
            if (modType == NULL)
                throw RedisException("Err couldn't create datatype");
            
        }
        
        virtual RedisModuleTypeMethods setMethods(RedisModuleCtx *ctx)
        {
            TypeSetup::setMethods<DataType>(ctx, name.c_str());
        }
        
        virtual std::map<std::string, RedisModuleCmdFunc> getCommands()=0;
        
        
    //private:
            
        RedisModuleType *modType;
        std::string name;
};

#endif /* BASEMODULE_H */

