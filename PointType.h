/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PointType.h
 * Author: jwhh9
 *
 * Created on June 26, 2018, 5:17 AM
 */

#ifndef POINTTYPE_H
#define POINTTYPE_H


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <array>
#include <sstream>
    
#include <boost/geometry/geometry.hpp>

    
#include "redismodule.h"

#include "Module/BaseModule.h"
#include "Module/CommandBuilder.h"
#include "Module/KeyValUtil.h"
#include "Module/ParseUtil.h"

#include "RedisException.h"

#include "FieldPoint.h"
#include "constants.h"
    
typedef bg::model::FieldPoint<double, POINT_DIMENSIONS, bg::cs::cartesian, SCALAR_FIELD_LENGTH> pt_type;
static RedisModuleType *PointModType;

using pt_cb = KeyValUtil::UserKey_CB<pt_type>;

class PointType: public BaseModule<pt_type>
{
public:
    PointType(): BaseModule("PointType") { }
    
    virtual RedisModuleTypeMethods setMethods(RedisModuleCtx *ctx) override
    {
        RedisModuleTypeMethods tm = BaseModule<pt_type>::setMethods(ctx);        
        tm.rdb_save = pt_type::save;
        tm.rdb_load = pt_type::load;
        
        return tm;
    }
    
    virtual std::map<std::string, RedisModuleCmdFunc> getCommands()
    {
        std::map<std::string, RedisModuleCmdFunc> map_cmd = {
            {"set", PointType::set_cmd},
            {"print", PointType::print_cmd}
        };

        return map_cmd;
    }
    
    static int set_cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        auto set_pt = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, pt_type *pPt)
        {
            pt_type::parse(ctx, argv, argc, pPt);   
            return RedisModule_ReplyWithNull(ctx);
        };
        
        return KeyValUtil::runCommand<pt_type, pt_type::ReqSize()>(ctx, argv, argc, PointModType, set_pt);        
    }
    
    static int print_cmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        auto print_pt = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, pt_type *pPt)
        {
            std::stringstream stream;
            *pPt << stream;
            
            return RedisModule_ReplyWithSimpleString(ctx, stream.str().c_str());
        };
        
        return KeyValUtil::runCommand<pt_type, ARG_COUNT_MIN>(ctx, argv, argc, PointModType, print_pt);        
    }
};

#endif /* POINTTYPE_H */

