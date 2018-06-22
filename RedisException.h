#ifndef REDIS_ECEPTION_H
#define REDIS_ECEPTION_H

#include <exception>
#include <string>

class RedisException : public std::exception
{
public:

    RedisException(const char *_msg) : std::exception()
    {
        msg = (char*) _msg;
    }
    
    RedisException(std::string _msg) : RedisException(_msg.c_str())
    { }

    virtual const char* what() const throw () override
    {
        return msg;
    }

protected:
    char *msg;
};

#endif