#include "redisvector.h"

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return vec.build(ctx, argv, argc);
}
