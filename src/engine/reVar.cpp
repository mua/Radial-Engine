#include "reVar.h"

reVar::~reVar()
{
	dataSize = 0;
}

bool reVar::operator==( const string &str )
{
	return str == asString();
}

void reVar::removeMember( const string& name )
{
	if (isMember(name))
	{
		membersMap.erase(name);
	}
}

void reVar::removeMember( int i )
{
	char number[10];
	_itoa_s(i, number, 10);
	removeMember(number);
	for ( size_t k=i; k<size()-1; k++)
	{
		(*this)[k] = (*this)[k+1];
	}
}

reVar& reVar::operator[]( const string& name )
{
	kind = seMAP; return membersMap[name];
}

reVar& reVar::operator[]( const int& i )
{
	char number[10];
	_itoa_s(i, number, 10);
	kind = seARRAY;
	return membersMap[number];
}

unsigned int reVar::size() const
{
	return membersMap.size();
}

reVar::reVar()
{
	dataSize = 0;
	data = 0;
	kind = seNULL;
}

reVar::reVar( const reVar& base ):
	dataSize(base.dataSize), 
	data(base.data), 
	membersMap(base.membersMap), 
	kind(base.kind)
{

}

reVar::reVar( const float data )
{
	set(data);
}

reVar::reVar( const int data )
{
	set(data);
}

reVar::reVar( const char* data )
{
	set(string(data));
}

reVar::reVar( const string data )
{
	set(data);
}

reVar::reVar( const Json::Value& value )
{
	data = 0;
	dataSize = 0;
	switch (value.type())
	{
	case Json::arrayValue:
		for (unsigned int i=0; i<value.size(); i++)
		{
			(*this)[i] = reVar(value[i]);
		}
		break;
	case Json::objectValue:
		for (unsigned int i=0; i<value.size(); i++)
		{
			string name = value.getMemberNames()[i];
			(*this)[name] = reVar(value[name]);
		}
		break;
	case Json::stringValue:
		set(value.asString());
		break;
	case Json::intValue:
	case Json::uintValue:
		set(value.asInt());
		break;
	case Json::realValue:
		set(value.asFloat());
		break;
	}
}

reVar::reVar( const reVec3& data )
{
	for (size_t i=0; i<3; i++)
	{
		(*this)[i] = data[i];
	}
}

reVar::reVar( const reVec4& data )
{
	for (size_t i=0; i<4; i++)
	{
		(*this)[i] = data[i];
	}
}

reVar& reVar::operator=( const reVar& base )
{
	dataSize = base.dataSize;
	data = base.data;
	membersMap = base.membersMap;
	kind = base.kind;
	return *this;
}

void reVar::set( const float data )
{
	this->data = shared_ptr<float>(new float(data));
	dataSize = sizeof(data);
	kind = seFLOAT;
}

void reVar::set( const string data )
{
	dataSize = data.length()+1;
	char *d  = new char[dataSize];
	this->data.reset(d);
	memcpy_s(d, dataSize, data.c_str(), dataSize);
	d[dataSize-1] = 0;
	kind = seSTRING;
}

void reVar::set( const int data )
{
	this->data = shared_ptr<int>(new int(data));
	dataSize = sizeof(data);
	kind = seINT;
}

bool reVar::isMember(const string& name) const
{
	return membersMap.find(name) != membersMap.end();
}

void reVar::append( reVar element )
{
	(*this)[membersMap.size()] = element;
}

float reVar::asFloat() const
{
	return *((const float*)data.get());
}

std::string reVar::asString() const
{
	return dataSize ? string((char*)data.get()) : "";
}

Json::Value reVar::asJson() const
{
	Json::Value value;
	switch (kind)
	{	
	case seNULL: 
		value = 0;
		break;
	case seFLOAT: 
		value = asFloat();
		break;
	case seINT: 
		value = asInt();
		break;
	case seSTRING: 
		value = asString();
		break;
	case seMAP:
		for (reVarMap::const_iterator it = membersMap.begin(); it != membersMap.end(); it++)
		{
			value[it->first] = it->second.asJson();
		}
		break;
	case seARRAY:
		value.resize(size());
		for (reVarMap::const_iterator it = membersMap.begin(); it != membersMap.end(); it++)
		{
			value[atoi(it->first.c_str())] = it->second.asJson();
		}
		break;
	}
	return value;
}

int reVar::asInt() const
{
	return *((const int*)data.get());
}

reVec3 reVar::asVec3() 
{
	reVec3 var;
	for (size_t i=0; i<3; i++)
	{
		var[i] = ((*this)[i]).asFloat();
	}
	return var;
}

reVec4 reVar::asVec4() 
{
	reVec4 var;
	for (size_t i=0; i<4; i++)
	{
		var[i] = ((*this)[i]).asFloat();
	}
	return var;
}
