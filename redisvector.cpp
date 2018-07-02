#include "redisvector.h"
#include "PointType.h"
#include "SimpleVec.h"

/******************************************************************************
REDIS VECTOR DEFS
 ******************************************************************************/

SimpleVec sv;
PointType ptMod;


/******************************************************************************
REDIS MODULE COMMANDS
 ******************************************************************************/

int RedisVector_InsertCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_AutoMemory(ctx);

    auto insert_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, ScalarCloud *pcloud)
    {
        if (argc != ScalarCloud::getInsertLength())
            throw RedisException("ERR wrong number of arguments for vector insert");
        
        pcloud->insert(ctx, argv, argc);
        RedisModule_ReplyWithNull(ctx);
    };

    return RedisVector_BaseCommand(ctx, argv, argc, insert_lam);
}

int RedisVector_SizeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    auto size_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, ScalarCloud *pcloud)
    {
        RedisModule_ReplyWithLongLong(ctx, pcloud->getSize());
    }; 

    return RedisVector_BaseCommand(ctx, argv, argc, size_lam);
}

int RedisVector_PrintCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    auto print_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, ScalarCloud *pcloud)
    {
        std::stringstream stream = pcloud->getFieldDataStream();
        RedisModule_ReplyWithSimpleString(ctx, stream.str().c_str());
    };

    return RedisVector_BaseCommand(ctx, argv, argc, print_lam);
}

int RedisVector_ClearCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    auto clear_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, ScalarCloud *pcloud)
    {
        pcloud->clear();
        RedisModule_ReplyWithNull(ctx);
    };

    return RedisVector_BaseCommand(ctx, argv, argc, clear_lam);
}

int RedisVector_NearestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    auto near_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, ScalarCloud *pcloud)
    {
        if (argc != POINT_QUERY_LENGTH)
            throw RedisException("ERR wrong number of arguments for point query");
        
        point_bg pt;
        
        parsePoint<POINT_DIMENSIONS - 1>(argv, pt);
        pcloud->printNearest(ctx, pt);
        
    };

    return RedisVector_BaseCommand(ctx, argv, argc, near_lam);
}

int RedisVector_SettreeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    auto settree_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, ScalarCloud *pcloud)
    {
        if (argc != ARG_COUNT_MIN)
            throw RedisException("ERR wrong number of arguments for setting rtree");
        
        pcloud->setRtree();
        RedisModule_ReplyWithSimpleString(ctx, "Rtree set");
    };

    return RedisVector_BaseCommand(ctx, argv, argc, settree_lam);
}

/******************************************************************************
REDIS SETUP
 ******************************************************************************/

RedisModuleTypeMethods RedisVector_MakeType(RedisModuleCtx *ctx)
{
    if (RedisModule_Init(ctx, cstr_redis_vect, 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
        throw RedisException("Err couldn't init module");
            
    RedisModuleTypeMethods tm;
    tm.version = REDISMODULE_TYPE_METHOD_VERSION;
    
    
    tm.rdb_load = [](RedisModuleIO *rdb, int encver)
    {
        ScalarCloud *pcloud = new ScalarCloud(); 
        pcloud->load(rdb); 
        return (void*) pcloud;
    };
    
    tm.rdb_save = [](RedisModuleIO *rdb, void *value)
    {
        ScalarCloud *pcloud = (ScalarCloud*) value; 
        pcloud->save(rdb); 
    };
    
    tm.aof_rewrite = [](RedisModuleIO *aof, RedisModuleString *key, void *value)
    {
        ScalarCloud *pcloud = (ScalarCloud*) value;
        RedisModule_EmitAOF(aof, "RedisVect.insert", "sl", key, "AOF not implemented");
    };
    
    tm.free = [](void *value)
    {
        ScalarCloud *pcloud = (ScalarCloud*) value; 
        delete pcloud;
    };
    
    return tm;
}

void RedisVector_SetCommands(RedisModuleCtx *ctx)
{
    std::vector<CommandBuilder> vec_cmd = 
    {
        CommandBuilder("RedisVect.insert", false, RedisVector_InsertCommand),
        CommandBuilder("RedisVect.print", true, RedisVector_PrintCommand),
        CommandBuilder("RedisVect.nearest", true, RedisVector_NearestCommand),
        CommandBuilder("RedisVect.size", true, RedisVector_SizeCommand),
        CommandBuilder("RedisVect.settree", true, RedisVector_SettreeCommand),
        CommandBuilder("RedisVect.clear", false, RedisVector_ClearCommand)
    };

    for (auto &cmd : vec_cmd)
        cmd.set(ctx);
}


/******************************************************************************
REDIS TYPE METHODS
 ******************************************************************************/

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    int status = REDISMODULE_OK;

    try
    {
        RedisModuleTypeMethods tm = RedisVector_MakeType(ctx);
        RedisVector_SetCommands(ctx);   
        
        RedisVector = RedisModule_CreateDataType(ctx, cstr_redis_vect, 0, &tm);
        if (RedisVector == NULL)
            throw RedisException("Err couldn't create datatype");
        
                

    } 
    catch (RedisException ex)
    {
        RedisModule_ReplyWithError(ctx, ex.what());
    }
    
    sv.initialize(ctx);
    ptMod.initialize(ctx);

    return status;
}
