#include "redisvector.h"

/******************************************************************************
REDIS VECTOR DEFS
 ******************************************************************************/



point_bg parsePoint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    point_bg pt;

    double coord;

    if (RedisModule_StringToDouble(argv[2], &coord) != REDISMODULE_OK)
        throw RedisException("ERR invalid value: must be integer");

    pt.set<0>(coord);

    if (RedisModule_StringToDouble(argv[3], &coord) != REDISMODULE_OK)
        throw RedisException("ERR invalid value: must be integer");

    pt.set<1>(coord);

    if (RedisModule_StringToDouble(argv[4], &coord) != REDISMODULE_OK)
        throw RedisException("ERR invalid value: must be integer");

    pt.set<2>(coord);

    return pt;
}

std::array<double, 1> parseArray(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    std::array<double, 1> ary;

    double val;
    if (RedisModule_StringToDouble(argv[argc - 1], &val) != REDISMODULE_OK)
        throw RedisException("ERR invalid value: must be integer");

    ary[0] = val;

    return ary;
}

/******************************************************************************
REDIS MODULE COMMANDS
 ******************************************************************************/

int RedisVector_InsertCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    try
    {
        RedisModule_AutoMemory(ctx);

        if (argc != ScalarCloud::getInsertLength())
            throw RedisException("ERR wrong number of arguments for vector insert");

        ScalarCloud *pcloud = ScalarCloud::getPointCloud(ctx, argv, argc, RedisVector);
        pcloud->insert(ctx, argv, argc);

        RedisModule_ReplyWithSimpleString(ctx, "Point added");
    } catch (RedisException exc)
    {
        return RedisModule_ReplyWithError(ctx, exc.what());
    }

    return REDISMODULE_OK;
}

int RedisVector_SizeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    try
    {
        ScalarCloud *pcloud = ScalarCloud::getPointCloud(ctx, argv, argc, RedisVector);
        RedisModule_ReplyWithLongLong(ctx, pcloud->getSize());
    } catch (RedisException exc)
    {
        return RedisModule_ReplyWithError(ctx, exc.what());
    }

    return REDISMODULE_OK;
}

int RedisVecot_PrintCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    try
    {
        ScalarCloud *pcloud = ScalarCloud::getPointCloud(ctx, argv, argc, RedisVector);
        std::stringstream stream = pcloud->getFieldDataStream();

        RedisModule_ReplyWithSimpleString(ctx, stream.str().c_str());

    } catch (RedisException exc)
    {
        return RedisModule_ReplyWithError(ctx, exc.what());
    }

    return REDISMODULE_OK;
}

int RedisVector_NearestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    try
    {
        RedisModule_AutoMemory(ctx);

        if (argc != POINT_QUERY_LENGTH)
            throw RedisException("ERR wrong number of arguments for point query");

        point_bg pt = parsePoint(ctx, argv, argc);

        ScalarCloud *pcloud = ScalarCloud::getPointCloud(ctx, argv, argc, RedisVector);

        std::stringstream stream;
        pcloud->appendNearestStream(stream, pt);

        RedisModule_ReplyWithSimpleString(ctx, stream.str().c_str());
    } catch (RedisException exc)
    {
        return RedisModule_ReplyWithError(ctx, exc.what());
    }

    return REDISMODULE_OK;
}

int RedisVector_SettreeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    try
    {
        RedisModule_AutoMemory(ctx);

        if (argc != ARG_COUNT_MIN)
            throw RedisException("ERR wrong number of arguments for setting rtree");

        ScalarCloud *pcloud = ScalarCloud::getPointCloud(ctx, argv, argc, RedisVector);
        pcloud->setRtree();

        RedisModule_ReplyWithSimpleString(ctx, "Rtree set");
    } catch (RedisException exc)
    {
        return RedisModule_ReplyWithError(ctx, exc.what());
    }

    return REDISMODULE_OK;
}

/******************************************************************************
REDIS TYPE METHODS
 ******************************************************************************/


void RedisVector_Save(RedisModuleIO *rdb, void *value)
{
    ScalarCloud *pcloud = (ScalarCloud*) value;
    pcloud->save(rdb);
}

void* RedisVector_Load(RedisModuleIO *rdb, int encver)
{
    ScalarCloud *pcloud = new ScalarCloud();
    pcloud->load(rdb);

    return pcloud;
}

void RedisVector_Rewrite(RedisModuleIO *aof, RedisModuleString *key, void *value)
{
    ScalarCloud *pcloud = (ScalarCloud*) value;

    RedisModule_EmitAOF(aof, "RedisVect.insert", "sl", key, "AOF not implemented");

}

void RedisVector_Free(void *value)
{
    ScalarCloud *pcloud = (ScalarCloud*) value;
    delete pcloud;
    //RedisModule_Free(value);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    int status = REDISMODULE_OK;

    try
    {
        if (RedisModule_Init(ctx, "RedisVect", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
            throw RedisException("Err couldn't init module");

        RedisModuleTypeMethods tm;
        tm.version = REDISMODULE_TYPE_METHOD_VERSION;
        tm.rdb_load = RedisVector_Load;
        tm.rdb_save = RedisVector_Save;
        tm.aof_rewrite = RedisVector_Rewrite;
        tm.free = RedisVector_Free;

        RedisVector = RedisModule_CreateDataType(ctx, "RedisVect", 0, &tm);
        if (RedisVector == NULL)
            throw RedisException("Err couldn't create datatype");

        if (RedisModule_CreateCommand(ctx, "RedisVect.insert",
                RedisVector_InsertCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
        {
            throw RedisException("Err couldn't create insert command");
        }

        if (RedisModule_CreateCommand(ctx, "RedisVect.size",
                RedisVector_SizeCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        {
            throw RedisException("Err couldn't create size command");
        }

        if (RedisModule_CreateCommand(ctx, "RedisVect.print",
                RedisVecot_PrintCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        {
            throw RedisException("Err couldn't create print command");
        }

        if (RedisModule_CreateCommand(ctx, "RedisVect.nearest",
                RedisVector_NearestCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        {
            throw RedisException("Err couldn't create nearest command");
        }

        if (RedisModule_CreateCommand(ctx, "RedisVect.settree",
                RedisVector_SettreeCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        {
            throw RedisException("Err couldn't create nearest command");
        }
    } catch (RedisException ex)
    {
        RedisModule_ReplyWithError(ctx, ex.what());
    }

    return status;
}
