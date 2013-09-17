#include "reProject.h"

reProject::reProject( const string& _root )
{
	root(_root);
}

// Getter - Setters

void reProject::root( const std::string& val )
{
	_root = val;
}

std::string reProject::root() const
{
	return _root;
}