/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ArgUtil.h
 * Author: jwhh9
 *
 * Created on July 1, 2018, 6:56 AM
 */

#ifndef ARGUTIL_H
#define ARGUTIL_H

#include <tuple>
#include <functional>

#include "../redismodule.h"
#include "ParseUtil.h"
#include "KeyValUtil.h"

namespace ArgUtil
{
    //RedisModuleCtx *ctx, RedisModuleString **argv, int argc
    typedef std::tuple<RedisModuleCtx*, RedisModuleString**, int> tup_mod_cmd;
    typedef std::function<int(RedisModuleCtx*, RedisModuleString**, int)> func_typ;
    
    
}

#endif /* ARGUTIL_H */

