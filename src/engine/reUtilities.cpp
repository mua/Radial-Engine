#include "reUtilities.h"

#include <algorithm>

using namespace std;

std::string strReplace(const std::string& str, const std::string& what, const std::string& with)
{
	size_t pos = 0;
	std::string ret(str);
	while ((pos = ret.find(what, pos)) != string::npos)
	{
		ret.replace(pos, what.length(), with);
		pos += with.length();
	}
	return ret;
}

std::string dirPath(string fileName)
{
	fileName = strReplace(fileName, "\\", "/");
	size_t pos = fileName.rfind("/");
	fileName.erase(fileName.begin() + pos, fileName.end());
	return fileName;
}

std::string baseName( std::string fileName )
{
	fileName = strReplace(fileName, "\\", "/");
	size_t pos = fileName.rfind("/");
	fileName.erase(fileName.begin(), fileName.begin() + pos);
	pos = fileName.rfind(".");
	fileName.erase(fileName.begin() + pos, fileName.end());
	return fileName;
}
