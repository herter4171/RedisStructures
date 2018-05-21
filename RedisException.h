#pragma once

#ifndef REDIS_ECEPTION_H
#define REDIS_ECEPTION_H

#include <exception>

class RedisException : public std::exception
{
public:
	RedisException(char *_msg) : std::exception()
	{
		msg = _msg;
	}

	virtual const char* what() const throw() override
	{
		return msg;
	}

protected:
	char *msg;
};

#endif