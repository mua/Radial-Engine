#include "reCollection.h"
#include "reRadial.h"
#include "reAssetLoader.h"
#include "reException.h"

reCollection::reCollection()
{

}

reCollection::reCollection( reCollection& from ): reObject(from)
{
	for (size_t i=0; i<from.count(); i++)
	{
		add(from.at(i)->clone());
	}
}

std::string reCollection::className()
{
	return "reCollection";
}

void reCollection::insert( reObject* object, int index )
{
	stringstream name; name << object->name(); int i = 0;
	while (true)
	{
		if (!objectByName(name.str())) break;
		i += 1; name.str(""); name << object->name() << "_" << i;
	}
	object->name(name.str());
	if (index>-1)
	{
		objects.insert(objects.begin()+index, object);
	} 
	else
	{
		objects.push_back(object);
	}	
	object->super = this;
}

void reCollection::remove( reObject* object )
{
	reObjectList::iterator it = find(objects.begin(), objects.end(), object);
	if (it != objects.end())
		objects.erase(it);
}

int reCollection::indexOf( reObject* object )
{
	reObjectList::iterator it = find(objects.begin(), objects.end(), object);
	return it != objects.end() ? it - objects.begin() : -1;
}

void reCollection::add( reObject* object, bool fixed )
{
	insert(object, objects.size());
	if (fixed)
	{
		fixedObjects.push_back(object);
	}
	object->afterAdd();
}

reObject* reCollection::objectByName(const std::string& name, bool recursive )
{
	if (name == this->name())
	{
		return this;
	}
	reCollection* sub;
	for ( size_t i=0; i<count(); i++)
	{
		if (recursive && (sub = dynamic_cast<reCollection*>(at(i))))
		{
			if (reObject* obj = sub->objectByName(name, true))
				return obj;
		}
		else
			if (at(i)->name() == name) return at(i);
	}
	return 0;
}

unsigned int reCollection::count() const
{
	return objects.size();
}

void reCollection::getChildrenJson(reVar& val)
{
	val["children"] = reVar();
	reVar& arr = val["children"];
	for ( size_t i=0; i<count(); i++)
	{	
		at(i)->toJson(arr[i]);
	}
}

void reCollection::toJson(reVar& val, int categories)
{
	reObject::toJson(val, categories);
	if (isReference())
	{
		return;
	}
	if (count() && categories & ALL_PROPERTIES)
	{
		getChildrenJson(val);
	}	
}

void reCollection::loadJson( reVar& value )
{
	reObject::loadJson(value);
	if (isReference())
	{
		return;
	}
	if (value.isMember("children"))
	{
		reObjectList list;
		unsigned int i;
		for (i = 0; i < value["children"].size(); i++)
		{
			reObject* obj = objectByName(value["children"][i]["name"].asString());
			if (obj)
				obj->loadJson(value["children"][i]);
			else
				obj = reRadial::shared()->assetLoader()->loadObject(value["children"][i]);
			list.push_back(obj);
		}
		for ( size_t i=objects.size()-1; i > -1; i--)
		{
			if (find(list.begin(), list.end(), objects[i]) == list.end() && find(fixedObjects.begin(), fixedObjects.end(), objects[i]) == fixedObjects.end())
			{
				delete objects[i];
			}
		}
		objects = fixedObjects;
		for (i=0; i<list.size(); i++)
		{
			if (find(fixedObjects.begin(), fixedObjects.end(), list[i]) == fixedObjects.end())
				add(list[i]);
		}	
	}
}

reObject* reCollection::at( int i )
{
	return objects[i];
}

reStringList reCollection::acceptedClasses()
{
	return reStringList();
}

void reCollection::clear()
{
	for ( size_t i=count()-1; i>=0; i++)
	{
		remove(at(i));
	}
}

bool reCollection::accepts(const string& className)
{
	reStringList list = acceptedClasses();
	return find(list.begin(), list.end(), className) != list.end();
}

reCollection::~reCollection()
{
	//indent++;
	while (count())
	{
		//at(i)->super = 0;
		delete at(0);
	}
	//indent--;
}

bool reCollection::isChild( reObject* obj )
{
	while (obj = obj->super)
		if (obj == this) return true;
	return false;
}

reCollection* reCollection::clone() 
{
	return  new reCollection(*this);
}

