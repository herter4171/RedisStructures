/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimpleVec.h
 * Author: jwhh9
 *
 * Created on July 1, 2018, 1:53 AM
 */

#ifndef SIMPLEVEC_H
#define SIMPLEVEC_H

#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <tuple>

#include "redismodule.h"
#include "constants.h"

#include "Module/KeyValUtil.h"
#include "Module/ParseUtil.h"
#include "Module/TypeSetup.h"
#include "Module/CommandBuilder.h"
#include "Module/ArgUtil.h"
#include "Module/BaseModule.h"


typedef std::vector<double> VecTyp;
RedisModuleType *SimpleVecModType;

class SimpleVec: public BaseModule<VecTyp>
{
    public:
        SimpleVec():BaseModule("SimpleVec")
        {
            modType = SimpleVecModType;
        }
        
        virtual std::map<std::string, RedisModuleCmdFunc> getCommands()
        {
            std::map<std::string, RedisModuleCmdFunc> map_cmd = {
                {"push", push_cmd},
                {"print", SimpleVec::print_cmd}
            };
            
            return map_cmd;
        }
        
        int push_cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
        {
            auto push = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, VecTyp *pVec)
            {
                double val;
                ParseUtil::parse(argv[argc - 1], val);
                pVec->push_back(val);

                RedisModule_ReplyWithNull(ctx);
            };
            
            KeyValUtil::runCommand<VecTyp, ARG_COUNT_MIN + 1>(ctx, argv, argc, SimpleVecModType, push);
            return REDISMODULE_OK;
        }
        
        static int print_cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
        {
            auto print = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, VecTyp *pVec)
            {
                std::stringstream stream;
                stream << "{ ";

                for (auto &val : *pVec)
                    stream << val << " ";

                stream << "}";
                RedisModule_ReplyWithSimpleString(ctx, stream.str().c_str());          
            };
            
            KeyValUtil::runCommand<VecTyp, ARG_COUNT_MIN>(ctx, argv, argc, SimpleVecModType, print);
            return REDISMODULE_OK;
        }
        
    private:
        
    
};


#endif /* SIMPLEVEC_H */
