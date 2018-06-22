#ifndef REDIS_VECTOR_H
#define REDIS_VECTOR_H

#include "RedisVec.h"

RedisVec<SCALAR_FIELD_LENGTH> vec;

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