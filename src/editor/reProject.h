#ifndef reProject_h__
#define reProject_h__

#include <string>

using namespace std;

class reProject
{
public:
	string _root;
	std::string root() const;
	void root(const string& val);

	reProject(const string& root);
};

#endif // reProject_h__