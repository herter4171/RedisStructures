#ifndef REDIS_VECTOR_H
#define REDIS_VECTOR_H

#include<vector>
#include<exception>
#include<string>
#include<sstream>
#include<array>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "redisalloc.h"
#include "redismodule.h"
#include "RedisException.h"
#include "RedisFieldPointCloud.h"
#include "FieldPoint.h"
#include "constants.h"

/******************************************************************************
REDIS VECTOR DEFS
 ******************************************************************************/

static RedisModuleType *RedisVector;

typedef bg::model::point<double, POINT_DIMENSIONS, bg::cs::cartesian> point_bg;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef RedisFieldPointCloud<SCALAR_FIELD_LENGTH> ScalarCloud;

/******************************************************************************
REDIS VECTOR FUNCTIONS
 ******************************************************************************/


point_bg parsePoint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

std::array<double, 1> parseArray(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);


/******************************************************************************
REDIS MODULE COMMANDS
 ******************************************************************************/

int RedisVector_InsertCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVector_SizeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVecot_PrintCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVector_NearestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int RedisVector_SettreeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);


#ifdef __cplusplus
extern "C"
{
#endif 

    void RedisVector_Save(RedisModuleIO *rdb, void *value);

    void* RedisVector_Load(RedisModuleIO *rdb, int encver);

    void RedisVector_Rewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);

    void RedisVector_Free(void *value);

    int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#ifdef __cplusplus
}
#endif
#endif