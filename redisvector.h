#ifndef REDIS_VECTOR_H
#define REDIS_VECTOR_H

#include<vector>
#include<exception>
#include<string>
#include<sstream>
#include<array>
#include <algorithm>
#include<tuple>
#include <functional>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "redisalloc.h"
#include "redismodule.h"
#include "RedisException.h"
#include "RedisFieldPointCloud.h"
#include "FieldPoint.h"
#include "constants.h"

#include "Module/CommandBuilder.h"

/******************************************************************************
REDIS VECTOR DEFS
 ******************************************************************************/

static RedisModuleType *RedisVector;

typedef bg::model::point<double, POINT_DIMENSIONS, bg::cs::cartesian> point_bg;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef RedisFieldPointCloud<SCALAR_FIELD_LENGTH> ScalarCloud;

const char *cstr_redis_vect = "RedisVect";


/******************************************************************************
REDIS VECTOR FUNCTIONS
 ******************************************************************************/


point_bg parsePoint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);


/******************************************************************************
REDIS MODULE COMMANDS
 ******************************************************************************/

template<typename F>
int RedisVector_BaseCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, F &func)
{
    try
    {
        ScalarCloud *pcloud = ScalarCloud::getPointCloud(ctx, argv, argc, RedisVector);
        func(ctx, argv, argc, pcloud);
    } 
    catch (RedisException exc)
    {
        return RedisModule_ReplyWithError(ctx, exc.what());
    }

    return REDISMODULE_OK;
}

template<typename F>
struct RedisVector_BaseCommandStruct
{
    
};

int RedisVector_InsertCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVector_SizeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVecot_PrintCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVector_NearestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVector_SettreeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

/******************************************************************************
REDIS SETUP
 ******************************************************************************/

RedisModuleTypeMethods RedisVector_MakeType(RedisModuleCtx *ctx);

void RedisVector_SetCommands(RedisModuleCtx *ctx);

#ifdef __cplusplus
extern "C"
{
#endif 
    
/******************************************************************************
REDIS TYPE METHODS
 ******************************************************************************/

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#ifdef __cplusplus
}
#endif
#endif