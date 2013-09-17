#ifndef reVal_h__
#define reVal_h__

#include <vector>
#include <map>
#include <memory>
#include "json/json.h"
#include "reTypes.h"

using  namespace std;

typedef shared_ptr<void> rePointer;

class reVar
{	
public:
	typedef map<string, reVar> reVarMap;
	typedef enum 
	{
		seNULL = 0,
		seFLOAT = 1,
		seINT = 2,
		seSTRING = 3,
		seMAP = 4,
		seARRAY = 5,
		seBINARY = 6
	} Kind;

	rePointer data;
	int dataSize;
	Kind kind;
	reVarMap membersMap;
	reVar& operator[](const string& name);
	reVar& operator[](const int& i);

	unsigned int size() const;

	reVar ();

	template <class T>
	reVar( T* data, int length )
	{
		set(data, length);
	}

	template <class T>
	void set( T* data, int length )
	{
		cout << sizeof(T);
		dataSize = length;
		this->data = shared_ptr<T>(data);
		kind = seBINARY;
	}

	~reVar ();

	reVar (const reVar& base); 

	reVar& operator = ( const reVar& base );

	reVar (const float data);

	reVar (const int data);

	reVar (const reVec3& data);
	reVar (const reVec4& data);

	reVar (const string data);

	reVar (const char* data);

	reVar (const Json::Value& value);

	void append(reVar element);

	bool isMember(const string& name) const;

	void set(const float data);

	void set(const int data);

	void set(const string data);

	float asFloat() const;

	int asInt() const;


	reVec3 asVec3();
	reVec4 asVec4();

	string asString() const;	

	virtual Json::Value asJson() const;
	bool reVar::operator==( const string &str );
	void removeMember( const string& name );
	void removeMember( int i);
};
#endif // reVal_h__