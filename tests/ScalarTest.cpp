#include <iostream>

#include <boost/thread.hpp>

#include "hiredis.h"
#include "async.h"
#include "adapters/libevent.h"

#include "AsyncTester.h"


#define REDIS_SERVER_HOST "127.0.0.1"
#define REDIS_SERVER_PORT 7000


int main(int argc, char** argv)
{
    std::vector<double> counts = {1E6};
    
    for (auto count : counts)
    {
        AsyncTester test1("TestKey2", REDIS_SERVER_HOST, REDIS_SERVER_PORT, count);
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

