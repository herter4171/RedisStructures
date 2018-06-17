/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PointStackParser.h
 * Author: jwhh9
 *
 * Created on June 17, 2018, 12:16 PM
 */

#ifndef POINTSTACKPARSER_H
#define POINTSTACKPARSER_H

#include <memory>

#include <boost/lockfree/stack.hpp>

#include "redismodule.h"

#include "FieldPoint.h"

template<std::size_t L>
class PointStackParser
{
public:
    typedef boost::lockfree::stack<RedisModuleString*> stack_redis_str;
    
    PointStackParser(std::shared_ptr<stack_redis_str> pStack_)
    {
        pStack = pStack_;
    }
    
private:
    
    std::shared_ptr<stack_redis_str> pStack;
};

#endif /* POINTSTACKPARSER_H */

