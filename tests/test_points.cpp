/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   test_points.cpp
 * Author: jwhh9
 *
 * Created on July 1, 2018, 5:52 PM
 */

#include <stdlib.h>
#include <iostream>

#include "AsyncPointFill.h"

#define REDIS_SERVER_HOST "127.0.0.1"
#define REDIS_SERVER_PORT 6379

int main(int argc, char** argv)
{
    
    std::vector<double> counts = {1E6};
    
    for (auto count : counts)
    {
        AsyncPointFill test1("PtSeq:", REDIS_SERVER_HOST, REDIS_SERVER_PORT, count);
        //AsyncTester test2("TestKey2", REDIS_SERVER_HOST, REDIS_SERVER_PORT, count);
        
        //boost::thread thread1(boost::ref(test1));
        //boost::thread thread2(boost::ref(test2));
        test1();
        //thread1.join();
        //thread2.join();
        std::cout << test1.getResult() << std:: endl << std::flush;
        //std::cout << test2.getResult() << std:: endl << std::flush;
    }

    return (EXIT_SUCCESS);
}

