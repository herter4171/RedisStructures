/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Deleter.h
 * Author: jwhh9
 *
 * Created on June 3, 2018, 1:27 PM
 */

#ifndef DELETER_H
#define DELETER_H

//extern void RedisModule_Free(void* value);

template<class T>
struct Deleter
{
    Deleter(){ }
    void operator()(T *ptr)
    {
        RedisModule_Free((void*) ptr);
    }
};

#endif /* DELETER_H */

