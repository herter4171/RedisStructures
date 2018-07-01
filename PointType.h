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

#include "Module/CommandBuilder.h"
#include "Module/KeyValUtil.h"
#include "Module/ParseUtil.h"

#include "RedisException.h"

#include "FieldPoint.h"
#include "constants.h"
    
typedef bg::model::FieldPoint<double, POINT_DIMENSIONS, bg::cs::cartesian, SCALAR_FIELD_LENGTH> pt_type;
using pt_cb = KeyValUtil::UserKey_CB<pt_type>;

static RedisModuleType *PointType;
const char *cstr_point_type = "PointType";


static pt_cb set_pt(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, pt_type *pPt)
{
    
}


int PointType_SetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{    
    /*const int req_argc = ARG_COUNT_MIN + POINT_DIMENSIONS + SCALAR_FIELD_LENGTH;
    
    KeyUtil::UserKey_CB<pt_type> parse = [](RedisModuleString **argv, int argc)
    {
        pt_type::Dim_Ary coords = {1, 2, 3};
        pt_type::Fld_Ary storage = {5};
        
        //ParseUtil::parse(coords, argv, ARG_COUNT_MIN);
        //ParseUtil::parse(storage, argv, ARG_COUNT_MIN + POINT_DIMENSIONS);
        
        return new pt_type(coords, storage);
    };
    
    KeyUtil::setValue<pt_type, 6>(ctx, argv, argc, PointType, parse);    
    RedisModule_ReplyWithNull(ctx);    */
    
    return REDISMODULE_OK;
}

int PointType_PrintCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    /*KeyUtil::UserKey_CB<pt_type> empty = [](RedisModuleString **argv, int argc){ pt_type *pPoint; return pPoint; };
    
    pt_type *ppoint = KeyUtil::fetchValue<pt_type, ARG_COUNT_MIN>(ctx, argv, argc, PointType, empty);   
    
    std::array<double, SCALAR_FIELD_LENGTH> scalar = ppoint->getStorage();    
    std::stringstream ptStream;
    
    ptStream << bg::get<0>(*ppoint) << " " << bg::get<1>(*ppoint) << " " << bg::get<2>(*ppoint) << " --> " << scalar[0] << std::endl;
    
    RedisModule_ReplyWithSimpleString(ctx, ptStream.str().c_str());*/
    
    return REDISMODULE_OK;
}

RedisModuleTypeMethods PointType_MakeType(RedisModuleCtx *ctx)
{
    if (RedisModule_Init(ctx, cstr_point_type,1,REDISMODULE_APIVER_1) == REDISMODULE_ERR) 
        throw RedisException("Err couldn't init module");
        
    RedisModuleTypeMethods tm;
    tm.version = REDISMODULE_TYPE_METHOD_VERSION;
    
    tm.rdb_load = pt_type::load;
    tm.rdb_save = pt_type::save;
    tm.aof_rewrite = [](RedisModuleIO *aof, RedisModuleString *key, void *value){ };
    
    tm.free = [](void *value)
    {
        pt_type *pPoint = (pt_type*) value;
        delete pPoint;
    };
    
    return tm;
}

void PointType_SetCommands(RedisModuleCtx *ctx)
{
    std::vector<CommandBuilder> vec_cmd = 
    {
        CommandBuilder("PointType.set", false, PointType_SetCommand),
        CommandBuilder("PointType.print", true, PointType_PrintCommand)
    };
    
    for (auto &cmd : vec_cmd)
    {
        cmd.set(ctx);
    }
}

int PointType_SetType(RedisModuleCtx *ctx)
{
    RedisModuleTypeMethods tm = PointType_MakeType(ctx);
    PointType_SetCommands(ctx);
    
    PointType = RedisModule_CreateDataType(ctx, cstr_point_type, 0, &tm);
    if (PointType == NULL)
        throw RedisException("Err couldn't create datatype");
    
    return REDISMODULE_OK;
}


#endif /* POINTTYPE_H */

