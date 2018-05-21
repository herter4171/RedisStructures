#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
#include <array>
#include <boost/timer/timer.hpp>

#include "hiredis.h"
#include "async.h"

std::default_random_engine generator;
std::uniform_real_distribution<double> dist(0.0, 1.0);

#define TEST_POINT_COUNT 1E6


const char *writeCstr = "RedisVect.insert TestKey %f %f %f %f";
const char *queryCstr = "RedisVect.nearest TestKey %f %f %f";

template<std::size_t L>
std::vector<std::array<double, L>>* getRandomValues(const int count)
{
    std::vector<std::array<double, L>> *pRandVals = new std::vector<std::array<double, L>>(count);
    
    for (auto it = pRandVals->begin(); it != pRandVals->end(); ++it)
    {
        for (auto &val : *it)
        {
            val = dist(generator);
        }
    }
    
    return pRandVals;
}

/*void dummyCallback(const redisAsyncContext *c, int status)
{
    
}*/

void testWritePoints(redisAsyncContext *ctx, std::vector<std::array<double, 4>> *pValVec)
{
    std::cout << "Begin test write for " << TEST_POINT_COUNT << " points... ";
    
    redisReply *r;
    boost::timer::auto_cpu_timer t;

    for (auto &ary : *pValVec)
    {
        //r = (redisReply*) redisCommand(ctx, writeCstr, ary[0], ary[1], ary[2], ary[3]);
        int res = redisAsyncCommand(ctx, NULL, NULL, writeCstr, ary[0], ary[1], ary[2], ary[3]);
        if (res == REDIS_ERR)
        {
            std::cout << "ERROR: Null result for point\n";
            break;
        }
    }
    
    
    delete pValVec;
}

void testQueryPoints(redisAsyncContext *ctx, std::vector<std::array<double, 3>> *pValVec)
{
    std::cout << "Begin test query for " << TEST_POINT_COUNT << " points... ";

    redisReply *r;
    boost::timer::auto_cpu_timer t;

    for (auto &ary : *pValVec)
    {
        //r = (redisReply*) redisCommand(ctx, queryCstr, ary[0], ary[1], ary[2]);
        int res =  redisAsyncCommand(ctx, NULL, NULL, queryCstr, ary[0], ary[1], ary[2]);
        if (res == REDIS_ERR)

        if (r == NULL)
        {
            std::cout << "ERROR: Null result for point\n";
            break;
        }
    }
    
}

int main(int argc, char** argv)
{
    //redisContext *ctx = redisConnect("127.0.0.1", 6379);
    redisAsyncContext *ctx = redisAsyncConnect("127.0.0.1", 6379);
    
    if (ctx == NULL || ctx->err) {
        if (ctx) {
            printf("Error: %s\n", ctx->errstr);
            // handle error
        } else {
            printf("Can't allocate redis context\n");
        }
    }
    else
    {
        std::vector<std::array<double, 4>> *pWriteVec = getRandomValues<4>(TEST_POINT_COUNT);
        testWritePoints(ctx, pWriteVec); 
        
        std::vector<std::array<double, 3>> *pQueryVec = getRandomValues<3>(TEST_POINT_COUNT);
        testQueryPoints(ctx, pQueryVec);
        
    }

    return (EXIT_SUCCESS);
}

