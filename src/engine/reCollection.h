#ifndef RECOLLECTION_H
#define RECOLLECTION_H

#include "reObject.h"
#include "reException.h"

#include <vector>
#include <algorithm>
#include <sstream>
#include <json/json.h>

using namespace std;

typedef vector<reObject*> reObjectList;
typedef vector<reCollection*> reCollectionList;

typedef std::vector<reObjectState*> reObjectStateList;

class reCollection: public reObject
{
public:
	reObjectList fixedObjects;
	reObjectList objects;	

	reCollection();
	reCollection(reCollection& from);
	~reCollection();
	virtual reCollection* clone() override;
	virtual string className();
	reObject* at(int i);
	virtual void insert(reObject* object, int index);
	virtual void remove(reObject* object);
	int indexOf(reObject* object);
	virtual void add(reObject* object, bool fixed = 0);
	unsigned int count() const;
	void clear();
	reObject* objectByName(const std::string& name, bool recursive = false);

	virtual void getChildrenJson(reVar& val);
	virtual void toJson(reVar& val, int categories = ALL_PROPERTIES) override;
	virtual void loadJson(reVar& value);

	virtual reStringList acceptedClasses();
	bool accepts(const string& className); 
	bool isChild(reObject* obj);
	template<class T> 
	T* findInSupers()
	{
		for (size_t i=0; i<count(); i++)
		{
			if (T* obj = dynamic_cast<T*>(at(i)))
			{
				return obj;
			}
		}
		return super ? super->findInSupers<T>() : 0;
	};
	template<class T> 
	void findObjects(std::vector<T*>& list, bool firstOnly = false)
	{
		if (T* obj = dynamic_cast<T*>(this)) list.push_back(obj);
		for (size_t i=0; i<count(); i++)
		{
			if (reCollection* coll = dynamic_cast<reCollection*>(at(i)))
			{
				coll->findObjects<T>(list);				
			} 
			else
			{
				if (T* obj = dynamic_cast<T*>(at(i)))
					list.push_back(obj);
			}
			if (list.size() && firstOnly) return;
		}
	}
	template<class T> 
	T* findObject()
	{
		std::vector<T*> list;
		findObjects(list, true);
		return list.size() ? list[0] : 0;
	}
};

template <class T>
class reTypedCollection: public reCollection
{
public:
	void insert( reObject* object, int index )
	{
		if (!dynamic_cast<T*>(object))
		{
			throw new reException("wrong kind of object inserted");
		}
		reCollection::insert(object, index);
	}
	T* at( int i )
	{
		return (T*)reCollection::at(i);
	}
};

#endif // RECOLLECTION_H
