#ifndef reException_h__
#define reException_h__

#include <exception>
#include <string>

class reException: public std::exception
{
private:
	std::string _message;
public:
	reException(std::string message);
	virtual const char* what() const throw();
};

#endif // reException_h__