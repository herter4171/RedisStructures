/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RedisVec.h
 * Author: jwhh9
 *
 * Created on June 21, 2018, 9:18 PM
 */

#ifndef REDISVEC_H
#define REDISVEC_H

#include <string>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "redismodule.h"
#include "Module/ModuleBuilder.h"
#include "RedisFieldPointCloud.h"
#include "Module/CommandBuilder.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

template<std::size_t L>
class RedisVec: public ModuleBuilder<RedisFieldPointCloud<L>>
{
public:
    static RedisModuleType *RedisVect;
    
    typedef bg::model::point<double, POINT_DIMENSIONS, bg::cs::cartesian> point_bg;
    typedef ModuleBuilder<RedisFieldPointCloud<L>> type_mbT;
    
    typedef std::function<void(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisFieldPointCloud<L>*)> func_type;

    
    RedisVec():ModuleBuilder<RedisFieldPointCloud<L>>("RedisVect", RedisVec::RedisVect)
    { }
    
    static int baseCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, 
            func_type &func)
    {
        try
        {
            RedisFieldPointCloud<L> *pcloud = type_mbT::getInstance(ctx, argv, argc, RedisVect);
            func(ctx, argv, argc, pcloud);
        } 
        catch (RedisException exc)
        {
            return RedisModule_ReplyWithError(ctx, exc.what());
        }

        return REDISMODULE_OK;
    }
    static int insert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        func_type insert_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisFieldPointCloud<L> *pcloud)
        {
            if (argc != RedisFieldPointCloud<L>::getInsertLength())
                throw RedisException("ERR wrong number of arguments");

            pcloud->insert(ctx, argv, argc);
            RedisModule_ReplyWithNull(ctx);
        };

        return baseCommand(ctx, argv, argc, insert_lam);
    }
    
    static int size(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        func_type size_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisFieldPointCloud<L> *pcloud)
        {
            RedisModule_ReplyWithLongLong(ctx, pcloud->getSize());
        }; 

        return baseCommand(ctx, argv, argc, size_lam);
    }

    static int print(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        func_type print_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisFieldPointCloud<L> *pcloud)
        {
            std::stringstream stream = pcloud->getFieldDataStream();
            RedisModule_ReplyWithSimpleString(ctx, stream.str().c_str());
        };

        return baseCommand(ctx, argv, argc, print_lam);
    }

    static int clear(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        func_type clear_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisFieldPointCloud<L> *pcloud)
        {
            pcloud->clear();
            RedisModule_ReplyWithNull(ctx);
        };

        return baseCommand(ctx, argv, argc, clear_lam);
    }

    static int nearest(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        func_type near_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisFieldPointCloud<L> *pcloud)
        {
            if (argc != POINT_QUERY_LENGTH)
                throw RedisException("ERR wrong number of arguments");

            point_bg pt = parsePoint(ctx, argv, argc);
            pcloud->printNearest(ctx, pt);

        };

        return baseCommand(ctx, argv, argc, near_lam);
    }

    static int setTree(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        func_type settree_lam = [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisFieldPointCloud<L> *pcloud)
        {
            if (argc != ARG_COUNT_MIN)
                throw RedisException("ERR wrong number of arguments");
            
            pcloud->setRtree();
            RedisModule_ReplyWithSimpleString(ctx, "Rtree set");
        };

        return baseCommand(ctx, argv, argc, settree_lam);
    }
    
protected:
    
    std::vector<CommandBuilder> getCommandBuilders()
    {
        return std::vector<CommandBuilder>
        {
            CommandBuilder("insert", false, RedisVec<L>::insert),
            CommandBuilder("print", true, RedisVec<L>::print),
            CommandBuilder("nearest", true, RedisVec<L>::nearest),
            CommandBuilder("size", true, RedisVec<L>::size),
            CommandBuilder("settree", true, RedisVec<L>::setTree),
            CommandBuilder("clear", false, RedisVec<L>::clear)
        };
    }
    
    RedisModuleTypeLoadFunc getLoader()
    {
        return [](RedisModuleIO *rdb, int encver)
        {
            RedisFieldPointCloud<L> *pcloud = new RedisFieldPointCloud<L>(); 
            pcloud->load(rdb); 
            return (void*) pcloud;
        };
    }
    
    RedisModuleTypeSaveFunc getSaver()
    {
        return [](RedisModuleIO *rdb, void *value)
        {
            RedisFieldPointCloud<L> *pcloud = (RedisFieldPointCloud<L>*) value; 
            pcloud->save(rdb); 
        };
    }
    
    RedisModuleTypeRewriteFunc getRewriter()
    {
        return [](RedisModuleIO *aof, RedisModuleString *key, void *value)
        {
            RedisFieldPointCloud<L> *pcloud = (RedisFieldPointCloud<L>*) value;
            RedisModule_EmitAOF(aof, "RedisVect.insert", "sl", key, "AOF not implemented");
        };
    }
    
    RedisModuleTypeFreeFunc getFreer()
    {
        return [](void *value)
        {
            RedisFieldPointCloud<L> *pcloud = (RedisFieldPointCloud<L>*) value; 
            delete pcloud;
        };
    }
        
private:
    
    
    static point_bg parsePoint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
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
};


#endif /* REDISVEC_H */

