#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
#include <array>
#include <signal.h>

#include <boost/timer/timer.hpp>
#include <boost/thread.hpp>

#include "hiredis.h"
#include "async.h"
#include "adapters/libevent.h"

#include "AsyncTester.h"

std::default_random_engine generator;
std::uniform_real_distribution<double> dist(0.0, 1.0);

#define REDIS_SERVER_HOST "192.168.1.232"
#define REDIS_SERVER_PORT 6379


const char *writeCstr = "RedisVect.insert TestKey %f %f %f %f";
const char *queryCstr = "RedisVect.nearest TestKey %f %f %f";
const char *clearCstr = "RedisVect.clear TestKey";

int eventCount, maxEvents;

redisAsyncContext* getContext(bool &valid, struct event_base *base)
{
    signal(SIGPIPE, SIG_IGN);
    
    redisAsyncContext *ctx = redisAsyncConnect(REDIS_SERVER_HOST, REDIS_SERVER_PORT);
    valid = !(ctx == NULL || ctx->err);
    
    if (valid)
    {
        redisLibeventAttach(ctx,base);
    }
    else
    {
        if (ctx) 
        {
            printf("Error: %s\n", ctx->errstr);
        } 
        else 
        {
            printf("Can't allocate redis context\n");
        }
    }
    
    return ctx;
}

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

void dummyCallback(redisAsyncContext *ctx, void *r, void *privdata) 
{
    redisReply *reply = (redisReply*) r;
    eventCount++;
    
    if (eventCount == maxEvents)
    {
        redisAsyncDisconnect(ctx);
    }
    return;
    //printf("argv[%s]: %s\n", (char*)privdata, reply->str);
}

void testWritePoints(std::vector<std::array<double, 4>> *pValVec)
{
    std::cout << "Begin test write for " << pValVec->size() << " points... " << std::flush;
    bool valid;
    struct event_base *base = event_base_new();
    
    redisAsyncContext *ctx = getContext(valid, base);
    
    if (valid)
    {
        redisReply *r;
        boost::timer::auto_cpu_timer t;

        for (auto &ary : *pValVec)
        {
            //r = (redisReply*) redisCommand(ctx, writeCstr, ary[0], ary[1], ary[2], ary[3]);
            eventCount = 0;
            int res = redisAsyncCommand(ctx, dummyCallback, NULL, writeCstr, ary[0], ary[1], ary[2], ary[3]);
            if (res == REDIS_ERR)
            {
                std::cout << "ERROR: Null result for point\n";
                break;
            }
        }
        event_base_dispatch(base);
    }
    
    delete pValVec;
    
}



void testQueryPoints(std::vector<std::array<double, 3>> *pValVec)
{
    std::cout << "Begin test query for " << pValVec->size() << " points... " << std::flush;
    
    bool valid;
    struct event_base *base = event_base_new();
    
    redisAsyncContext *ctx = getContext(valid, base);
    
    
    if (valid)
    {
        redisReply *r;
        boost::timer::auto_cpu_timer t;

        for (auto &ary : *pValVec)
        {
            //r = (redisReply*) redisCommand(ctx, queryCstr, ary[0], ary[1], ary[2]);
            eventCount = 0;
            int res =  redisAsyncCommand(ctx, dummyCallback, NULL, queryCstr, ary[0], ary[1], ary[2]);
            if (res == REDIS_ERR)
            {
                std::cout << "ERROR: Null result for point\n";
                break;
            }
        }
        
        event_base_dispatch(base);
    }
    delete pValVec;
    
}

int main(int argc, char** argv)
{
    std::vector<double> counts = {1E2};
    
    for (auto count : counts)
    {
        AsyncTester test1("TestKey1", REDIS_SERVER_HOST, REDIS_SERVER_PORT, count);
        //AsyncTester test2("TestKey2", REDIS_SERVER_HOST, REDIS_SERVER_PORT, count);
        
        boost::thread thread1(boost::ref(test1));
        //boost::thread thread2(boost::ref(test2));
        
        thread1.join();
        //thread2.join();
        std::cout << test1.getResult() << std:: endl << std::flush;
        //std::cout << test2.getResult() << std:: endl << std::flush;
    }

    return (EXIT_SUCCESS);
}

