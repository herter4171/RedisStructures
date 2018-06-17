#include <memory>
#include <vector>
#include <array>
#include <sstream>
#include <cstdlib>
#include <atomic>
#include <string>
#include <unistd.h>

#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/spirit/include/qi.hpp>

#include "redisalloc.h"
#include "redismodule.h"

#include "constants.h"
#include "RedisException.h"
#include "FieldPoint.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

template<std::size_t L>
class RedisFieldPointCloud
{
public:
    typedef bg::model::point<double, POINT_DIMENSIONS, bg::cs::cartesian> point_bg;
    
    typedef bg::model::FieldPoint<double, POINT_DIMENSIONS, bg::cs::cartesian, std::array<double, L>> field_pt;

    typedef std::vector<field_pt, RedisAlloc<field_pt>> vec_fields;
    typedef bgi::rtree<field_pt, bgi::quadratic < 16 >, bgi::indexable<field_pt>, bgi::equal_to<field_pt>, RedisAlloc<field_pt>> rtree_fields;
    typedef boost::lockfree::queue<std::string*> queue_redis_str;

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
        pInsertThread = std::shared_ptr<boost::thread>(new boost::thread(boost::ref(ptParse)));
        pVecField = ptParse.pVecField;
    }

    void setRtree()
    {
        ptParse.finished = true;
        
        while (!ptParse.pQueue->empty())
        {
            usleep(1000);
        }
        
        pRtreeField = std::make_shared<rtree_fields>(ptParse.pVecField->begin(), ptParse.pVecField->end());
    }

    void insert(RedisModuleCtx *ctx, RedisModuleString **argv, const int argc)
    {
        //pVecField->push_back(parsePoint(ctx, argv, argc));
        ptParse.enqueue(ctx, argv, ARG_COUNT_MIN);
    }

    long long getSize()
    {
        return pVecField->size();
    }

    void appendNearestStream(std::stringstream &stream, point_bg pt)
    {
        field_pt nearField = getNearestField(pt);
        appendFieldToStream(stream, nearField);
    }
    
    void printNearest(RedisModuleCtx *ctx, point_bg pt)
    {
        field_pt nearField = getNearestField(pt);
        
        RedisModule_ReplyWithDouble(ctx, nearField.getStorage()[0]);
    }
    
    std::stringstream getNearestPointString(point_bg pt)
    {
        std::stringstream stream;
        appendNearestStream(stream, pt);
        
        return stream;        
    }

    std::stringstream getFieldDataStream()
    {
        std::stringstream stream;

        for (auto &pt : *pVecField)
        {
            appendFieldToStream(stream, pt);
        }

        return stream;
    }

    void save(RedisModuleIO *rdb)
    {
        uint64_t size = pVecField->size();

        for (auto &pt : *pVecField)
        {
            RedisModule_SaveDouble(rdb, bg::get<0>(pt));
            RedisModule_SaveDouble(rdb, bg::get<1>(pt));
            RedisModule_SaveDouble(rdb, bg::get<2>(pt));

            for (const auto &val : pt.getStorage())
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
            
            std::array<double, POINT_DIMENSIONS> coords;
            std::array<double, L> storage;
            
            std::copy(allAry.begin(), allAry.begin() + POINT_DIMENSIONS, coords.begin());
            std::copy(allAry.begin() + POINT_DIMENSIONS, allAry.end(), storage.begin());            

            pVecField->push_back(field_pt(coords, storage));
        }
    }
    
    void clear()
    {
        pVecField->clear();
        pRtreeField->clear();
    }

private:
    struct PointStackParser
    {
        PointStackParser()
        {
            pQueue = std::make_shared<queue_redis_str>(0);
            pVecField = std::make_shared<vec_fields>();
            
            finished = false;
        }
        
        void enqueue(RedisModuleCtx *ctx, RedisModuleString **argv, int offset)
        {
            std::string *pStr = new std::string[L + POINT_DIMENSIONS];
            
            // Keep strings in memory
            for (int i = 0; i < L + POINT_DIMENSIONS; i++)
            {
                std::size_t len;
                const char *cstr = RedisModule_StringPtrLen(argv[i + offset], &len);
                pStr[i] = std::string(cstr, len);
            }
            
            pQueue->push(pStr);
        }
        
        int operator()()
        {
            while(!finished || !pQueue->empty())
            {
                std::string *aryStr;
                
                if (pQueue->pop(aryStr))
                {                   
                    pVecField->push_back(parsePoint(aryStr));
                }
                else
                {
                    usleep(1000);
                }
                    
            }
        }
        
        field_pt parsePoint(std::string *aryStr)
        {
            std::array<double, POINT_DIMENSIONS> valAry;
            std::array<double, L> dataAry;
            
            for (int i = 0; i < POINT_DIMENSIONS; i++)
            {
                valAry[i] = std::stod(aryStr[i]);
            }
            
            for (int i = 0; i < L; i++)
            {
                dataAry[i] = std::stod(aryStr[i + POINT_DIMENSIONS]);
            }
            
            //delete aryStr;

            return field_pt(valAry, dataAry);
        }
        
        std::shared_ptr<queue_redis_str> pQueue;
        std::shared_ptr<vec_fields> pVecField;
        
        std::atomic_bool finished;
    };
    

    
    std::shared_ptr<vec_fields> pVecField;
    std::shared_ptr<rtree_fields> pRtreeField;
    std::shared_ptr<boost::thread> pInsertThread;
    
    PointStackParser ptParse;
    

    field_pt getNearestField(point_bg pt)
    {
        vec_fields result;

        if (!pRtreeField || pRtreeField->empty())
            setRtree();

        pRtreeField->query(bgi::nearest(pt, 1), std::back_inserter(result));
        
        if (result.empty())
        {
            throw RedisException("ERR empty result!  Have any points been added?");
        }

        return result.front();
    }

    void appendFieldToStream(std::stringstream &stream, field_pt &pt)
    {
        stream << bg::wkt(pt) << " --> ";
        
        for (auto &val : pt.getStorage())
        {
            stream << val << " ";
        }

        stream << std::endl;
    }

    field_pt parsePoint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        std::array<double, POINT_DIMENSIONS> valAry = parseDoubles<POINT_DIMENSIONS>(ctx, argv, argc, ARG_COUNT_MIN);
        std::array<double, L> dataAry = parseDoubles<L>(ctx, argv, argc, argc - L);
        
        return field_pt(valAry, dataAry);
    }

    template<std::size_t Ary_Size>
    std::array<double, Ary_Size> parseDoubles(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, const int startInd)
    {
        std::array<double, Ary_Size> valAry;
        int argInd = startInd;
        
        using boost::spirit::qi::double_;
        using boost::spirit::qi::parse;
        

        for (auto &val : valAry)
        {
            if (RedisModule_StringToDouble(argv[argInd], &val) != REDISMODULE_OK)
                throw RedisException("ERR invalid point double");
            argInd++;
        }

        return valAry;
    }
};