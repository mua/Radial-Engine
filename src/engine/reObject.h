#ifndef reObject_h__
#define reObject_h__

#include <string>
#include <iostream>

#include "reVar.h"
#include "reTypes.h"

using namespace std;

class reObject;
class reCollection;

extern int indent, ii;

class reObjectState
{
public:
	string name;
	reCollection* super;
};

class reObject
{
private:
	string _name;
	string _path;
	bool _isReference;
public:
	enum rePropertyCategories
	{
		ALL_PROPERTIES = 1,
		LEAF_PROPERTIES = 2
	};

	reCollection* super;
	reObject();
	virtual ~reObject();
	reObject(reObject& from);
	virtual string className();
	virtual void toJson(reVar& val, int categories = 1);
	virtual void loadJson(reVar& value);

	virtual void saveToFile(string fileName);
	virtual void loadFromFile(string fileName);

	virtual reVar state();
	virtual void state(reVar state_);	
	
	virtual std::string toString();
	virtual void afterAdd();
public:
	reCollection* root();
	string name();
	void name(string aname);
	std::string path() const;
	void path(std::string val);
	bool fixed();
	bool isReference() const;
	void isReference(bool val);
	virtual reObject* clone();
	virtual reObject* copy();
};

#endif // reObject_h__
