#ifndef reUtilities_h__
#define reUtilities_h__

#define membersize(type, member) sizeof(((type *)0)->member)

#include <string>

std::string strReplace(const std::string& str, const std::string& what, const std::string& with);
std::string dirPath(std::string fileName);
std::string baseName(std::string fileName);

#endif // reUtilities_h__