#ifndef reRefNode_h__
#define reRefNode_h__

#include "reCollection.h"

class reRefNode:
	public reCollection
{
public:
	std::string _fileName;
	std::string fileName() const { return _fileName; }
	void fileName(std::string val);

	virtual reVar getChildrenJson();
	virtual void toJson(reVar& val);
	virtual void loadJson(reVar& val);
	void loadNodeFile(std::string fileName);
	virtual string className();
//	virtual void save(string& fileName);
//	virtual void load(string& fileName);

};

#endif // reRefNode_h__