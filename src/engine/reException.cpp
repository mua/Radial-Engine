#include "reException.h"

reException::reException( std::string message )
{
	_message = message;
}

const char* reException::what() const throw()
{
	return _message.c_str();
}