#include <memory>
#include <vector>
#include <array>
#include <sstream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "redisalloc.h"
#include "redismodule.h"

#include "constants.h"
#include "RedisException.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

template<std::size_t L>
class RedisFieldPointCloud
{
public:
    typedef bg::model::point<double, POINT_DIMENSIONS, bg::cs::cartesian> field_pt;
    typedef std::pair<field_pt, std::array<double, L>> field_pair;

    typedef std::vector<field_pair> vec_fields;
    typedef bgi::rtree<field_pair, bgi::quadratic < 16 >> rtree_fields;

    static RedisFieldPointCloud* getPointCloud(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, RedisModuleType *ptype)
    {
        if (argc < ARG_COUNT_MIN)
            throw RedisException("ERR wrong number of arguments!");

        RedisModuleKey *key = (RedisModuleKey*) RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
        int type = RedisModule_KeyType(key);

        RedisFieldPointCloud *pcloud;
        if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != ptype)
            throw RedisException(REDISMODULE_ERRORMSG_WRONGTYPE);
        else if (type == REDISMODULE_KEYTYPE_EMPTY)
        {
            pcloud = new RedisFieldPointCloud(); // Using RedisAlloc<T> is a bad time here
            RedisModule_ModuleTypeSetValue(key, ptype, pcloud);
        } else
        {
            pcloud = (RedisFieldPointCloud*) RedisModule_ModuleTypeGetValue(key);
        }

        return pcloud;
    }

    static const std::size_t getInsertLength()
    {
        return POINT_QUERY_LENGTH + L;
    }

    RedisFieldPointCloud()
    {
        pVecField = std::make_shared<vec_fields>();
    }

    void setRtree()
    {
        pRtreeField = std::make_shared<rtree_fields>(pVecField->begin(), pVecField->end());
    }

    void insert(RedisModuleCtx *ctx, RedisModuleString **argv, const int argc)
    {
        field_pt pt = parsePoint(ctx, argv, argc);
        std::array<double, L> ary = parseDoubles<L>(ctx, argv, argc, argc - L);

        pVecField->push_back(field_pair(pt, ary));
    }

    long long getSize()
    {
        return pVecField->size();
    }

    void appendNearestStream(std::stringstream &stream, field_pt pt)
    {
        field_pair nearField = getNearestField(pt);
        appendFieldToStream(stream, nearField);
    }

    std::stringstream getFieldDataStream()
    {
        std::stringstream stream;

        for (const auto &pair : *pVecField)
        {
            appendFieldToStream(stream, pair);
        }

        return stream;
    }

    void save(RedisModuleIO *rdb)
    {
        uint64_t size = pVecField->size();

        for (const auto &pair : *pVecField)
        {
            field_pt pt = pair.first;

            RedisModule_SaveDouble(rdb, pt.get<0>());
            RedisModule_SaveDouble(rdb, pt.get<1>());
            RedisModule_SaveDouble(rdb, pt.get<2>());

            for (const auto &val : pair.second)
            {
                RedisModule_SaveDouble(rdb, val);
            }
        }
    }

    void load(RedisModuleIO *rdb)
    {
        uint64_t size = RedisModule_LoadUnsigned(rdb);

        for (int i = 0; i < size; i++)
        {
            std::array<double, POINT_DIMENSIONS + L> allAry;

            for (auto &val : allAry)
            {
                val = RedisModule_LoadDouble(rdb);
            }

            field_pt pt(allAry[0], allAry[1], allAry[2]);
            std::array<double, L> fldAry;

            int ind = 0;
            for (auto it = allAry.begin() + POINT_DIMENSIONS; it != allAry.end(); ++it)
            {
                fldAry[ind] = *it;
                ind++;
            }

            pVecField->push_back(std::make_pair(pt, fldAry));
        }
    }

private:
    std::shared_ptr<vec_fields> pVecField;
    std::shared_ptr<rtree_fields> pRtreeField;

    field_pair getNearestField(field_pt pt)
    {
        vec_fields result;

        if (!pRtreeField)
            setRtree();

        pRtreeField->query(bgi::nearest(pt, 1), std::back_inserter(result));

        return result.front();
    }

    void appendFieldToStream(std::stringstream &stream, const field_pair &pair)
    {
        stream << bg::wkt(pair.first) << " --> ";

        for (int i = 0; i < L; i++)
        {
            stream << pair.second[i] << " ";
        }

        stream << std::endl;
    }

    field_pt parsePoint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        std::array<double, POINT_DIMENSIONS> valAry = parseDoubles<POINT_DIMENSIONS>(ctx, argv, argc, ARG_COUNT_MIN);
        return field_pt(valAry[0], valAry[1], valAry[2]);
    }

    template<std::size_t Ary_Size>
    std::array<double, Ary_Size> parseDoubles(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, const int startInd)
    {
        std::array<double, Ary_Size> valAry;
        int argInd = startInd;

        for (auto &val : valAry)
        {
            if (RedisModule_StringToDouble(argv[argInd], &val) != REDISMODULE_OK)
                throw RedisException("ERR invalid point double");
            argInd++;
        }

        return valAry;
    }
};