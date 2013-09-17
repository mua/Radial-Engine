#include "reObject.h"
#include "reCollection.h"

#include <sstream>
#include "reRadial.h"

#include "reAssetLoader.h"
#include <fstream>
#include <iostream>

int indent = 0;
int ii = 0;
int acount = 0;

reObject::reObject()
{
	//std::cout << "new object :" << acount++ << std::endl;
	super = 0;
	_isReference = false;
}

reObject::reObject( reObject& from )
{
	path(from.path());
	name(from.name());
	isReference(from.isReference());
}

std::string reObject::className()
{
	return "reObject";
}

void reObject::toJson(reVar& val, int categories)
{
	val["className"] = className();
	val["path"] = path();
	val["name"] = name();
	val["isReference"] = isReference();
}

void reObject::loadJson( reVar& value )
{
	if (value.isMember("path") && !value["path"].asString().empty())
	{
		loadFromFile(value["path"].asString());
	}
	if (value.isMember("name"))
	{
		name(value["name"].asString());
	}
	isReference((value.isMember("isReference") && value["isReference"].asInt()) || !value["path"].asString().empty());
}

std::string reObject::name()
{
	return _name.size() ? _name : className();
}

void reObject::name( string aname )
{
	_name = aname;
}

reVar reObject::state()
{
	reVar state;
	toJson(state);
	return state;
}

void reObject::state( reVar state_ )
{
	loadJson(state_);
}

std::string reObject::toString()
{
	reVar json; toJson(json);
	return json.asJson().toStyledString();
}

void reObject::saveToFile( string fileName )
{
	std::ofstream stream(fileName, ios::out);
	reVar json; 
	toJson(json);
	path(fileName);
	json["path"] = path();
	std::string s = json.asJson().toStyledString();
	stream << s;
	stream.close();
}

void reObject::loadFromFile( string fileName )
{
	Json::Value val;
	Json::Reader reader;
	reader.parse(reRadial::shared()->assetLoader()->loadFile(fileName), val);
	path(fileName);
	val.removeMember("path");	
	loadJson(reVar(val));
}

std::string reObject::path() const
{
	return _path;
}

void reObject::path( std::string val )
{
	_path = reRadial::shared()->assetLoader()->relativePath(val);
}

bool reObject::fixed()
{
	return super && find(super->fixedObjects.begin(), super->fixedObjects.end(), this) != super->fixedObjects.end();
}

reObject::~reObject()
{
	/*
	for ( size_t i=0; i<indent; i++)
	{
		std::cout << "   ";
	}
	std::cout << "freeing: " << this << " " << name() << " " << ii++ << std::endl;
	*/
	if (super)
	{
		super->remove(this);
	}
	//std::cout << "new object :" << acount-- << std::endl;
}

void reObject::afterAdd()
{

}

reCollection* reObject::root()
{
	reCollection* s = super;
	while (s->super)
		s = s->super;
	return s;
}

bool reObject::isReference() const
{
	return _isReference;
}

void reObject::isReference( bool val )
{
	_isReference = val;
}

reObject* reObject::clone()
{
	return new reObject(*this);
}

reObject* reObject::copy()
{
	reVar val;
	toJson(val);
	return reRadial::shared()->assetLoader()->loadObject(val);
}
