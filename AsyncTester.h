/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AsyncTester.h
 * Author: jwhh9
 *
 * Created on June 3, 2018, 3:58 PM
 */

#ifndef ASYNCTESTER_H
#define ASYNCTESTER_H

#include <string>
#include <sstream>
#include <signal.h>
#include <random>
#include <array>

#include <boost/timer/timer.hpp>

#include "hiredis.h"
#include "async.h"
#include "adapters/libevent.h"

class AsyncTester
{
public:
    AsyncTester(const char *key_, const char *host_, const int port_, const int size): 
        key(key_), host(host_), port(port_)
    {
        dist = std::uniform_real_distribution<double> (0.0, 1.0);
        num_points = size;
    }
        
    static void callback(redisAsyncContext *ctx, void *r, void *privdata) 
    {
        redisReply *reply = (redisReply*) r;
        AsyncTester *instTester = (AsyncTester*) privdata;
        instTester->event_count++;

        if (instTester->event_count == instTester->num_points)
        {
            redisAsyncDisconnect(ctx);
        }
        return;
    }
    
    int operator()()
    {
        event_count = 0;
        std::vector<std::array<double, 4>> *pWriteVec = getRandomValues<4>(num_points);
        testWritePoints(pWriteVec); 

        event_count = 0;
        std::vector<std::array<double, 3>> *pQueryVec = getRandomValues<3>(num_points);
        testQueryPoints(pQueryVec);
    }
    
    std::string getResult()
    {
        return msg_stream.str();
    }
        
    redisAsyncContext* getContext(bool &valid, struct event_base *base)
    {
        signal(SIGPIPE, SIG_IGN);

        redisAsyncContext *ctx = redisAsyncConnect(host, port);
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
    
    void testWritePoints(std::vector<std::array<double, 4>> *pValVec)
    {
        msg_stream << "Begin test write for " << pValVec->size() << " points... ";
        bool valid;
        struct event_base *base = event_base_new();

        redisAsyncContext *ctx = getContext(valid, base);
        
        std::string writeStr = pfx;
        writeStr += "insert ";
        writeStr += key;
        writeStr += " %f %f %f %f";
        
        const char *writeCstr = writeStr.c_str();

        if (valid)
        {
            redisReply *r;
            boost::timer::auto_cpu_timer t(msg_stream);

            for (auto &ary : *pValVec)
            {
                //r = (redisReply*) redisCommand(ctx, writeCstr, ary[0], ary[1], ary[2], ary[3]);
                
                int res = redisAsyncCommand(ctx, callback, this, writeCstr, ary[0], ary[1], ary[2], ary[3]);
                if (res == REDIS_ERR)
                {
                    msg_stream << "ERROR: Null result for point\n";
                    break;
                }
            }
            event_base_dispatch(base);
        }

        delete pValVec;

    }
    
    void testQueryPoints(std::vector<std::array<double, 3>> *pValVec)
    {
        msg_stream << "Begin test query for " << pValVec->size() << " points... ";

        bool valid;
        struct event_base *base = event_base_new();

        redisAsyncContext *ctx = getContext(valid, base);
        
        std::string queryStr = pfx;
        queryStr += "nearest ";
        queryStr += key;
        queryStr += " %f %f %f";
        
        const char *queryCstr = queryStr.c_str();

        if (valid)
        {
            redisReply *r;
            boost::timer::auto_cpu_timer t(msg_stream);

            for (auto &ary : *pValVec)
            {
                //r = (redisReply*) redisCommand(ctx, queryCstr, ary[0], ary[1], ary[2]);
                int res =  redisAsyncCommand(ctx, callback, this, queryCstr, ary[0], ary[1], ary[2]);
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
    
private:
    const char *pfx = "RedisVect.", *host, *key;
    const int port;
    
    
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist;
    
    std::stringstream msg_stream;
    
    int num_points, event_count;
};


#endif /* ASYNCTESTER_H */

