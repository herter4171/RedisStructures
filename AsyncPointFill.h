/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AsyncPointFill.h
 * Author: jwhh9
 *
 * Created on July 1, 2018, 5:53 PM
 */

#ifndef ASYNCPOINTFILL_H
#define ASYNCPOINTFILL_H


#include <string>
#include <sstream>
#include <signal.h>
#include <random>
#include <array>

#include <boost/timer/timer.hpp>

#include "hiredis.h"
#include "async.h"
#include "adapters/libevent.h"

class AsyncPointFill
{
public:
    AsyncPointFill(const char *key_, const char *host_, const int port_, const int size): 
        key_pfx(key_), host(host_), port(port_)
    {
        dist = std::uniform_real_distribution<double> (0.0, 1.0);
        num_points = size;
    }
        
    static void callback(redisAsyncContext *ctx, void *r, void *privdata) 
    {
        redisReply *reply = (redisReply*) r;
        AsyncPointFill *instTester = (AsyncPointFill*) privdata;
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
        write(pWriteVec); 

        /*event_count = 0;
        std::vector<std::array<double, 3>> *pQueryVec = getRandomValues<3>(num_points);
        query(pQueryVec);*/
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
    
    void write(std::vector<std::array<double, 4>> *pValVec)
    {
        msg_stream << "Begin test write for " << pValVec->size() << " points... ";
        bool valid;
        struct event_base *base = event_base_new();

        redisAsyncContext *ctx = getContext(valid, base);
        
        std::string cmd_pfx = mod_pfx;
        cmd_pfx += "set ";
        cmd_pfx += key_pfx;
        
        std::string cmd_sfx = " %f %f %f %f";
        int ind = 0;
        
        auto cmd_fetch = [](std::string &pfx, std::string &sfx, int ind)
        {
            return pfx + std::to_string(ind) + sfx;
        };

        if (valid)
        {
            redisReply *r;
            boost::timer::auto_cpu_timer t(msg_stream);

            for (auto &ary : *pValVec)
            {
                //r = (redisReply*) redisCommand(ctx, writeCstr, ary[0], ary[1], ary[2], ary[3]);
                std::string cmd = cmd_fetch(cmd_pfx, cmd_sfx, ind);
                
                int res = redisAsyncCommand(ctx, callback, this, cmd.c_str(), ary[0], ary[1], ary[2], ary[3]);
                if (res == REDIS_ERR)
                {
                    msg_stream << "ERROR: Null result for point\n";
                    break;
                }
                
                ind++;
            }
            event_base_dispatch(base);
        }

        delete pValVec;

    }
    
private:
    const char *mod_pfx = "PointType.", *host, *key_pfx;
    const int port;
    
    
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist;
    
    std::stringstream msg_stream;
    
    int num_points, event_count;
};



#endif /* ASYNCPOINTFILL_H */

