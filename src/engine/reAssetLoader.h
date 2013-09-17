#ifndef reAssetLoader_h__
#define reAssetLoader_h__

#include <json/json.h>
#include "reNode.h"
#include "reVar.h"

#include <string>
#include <map>

class reImage;

struct reObjectEntry
{
	reObject* object;
	int retainCount;
	reObjectEntry();
	reObjectEntry(reObject* obj, int retainCount = 0);
};

struct reClassEntryBase
{
	virtual reObject* instance() { return 0; };
};

template<class T>
struct reClassEntry: public reClassEntryBase
{
	virtual reObject* instance()
	{
		return new T;
	}
};

typedef std::map<std::string, reObjectEntry> reFileObjectMap;
typedef std::map<std::string, reClassEntryBase*> reClassMap;

class reAssetLoader
{	
public:	
	reStringList paths;
	reFileObjectMap sharedObjects;
	reClassMap classes;

	reAssetLoader();

	virtual std::string filePath(std::string fileName);
	virtual std::string relativePath(std::string fileName);

	virtual void loadImage( std::string& fileName, reImage* out);
	virtual reObject* loadObject (reVar& var);
	virtual reObject* loadObject (std::string& fileName, bool asRef = false);
	void loadObject (reObject* obj, std::string& fileName);	
	virtual std::string loadFile(std::string& fileName);

	void addPath( std::string val );
	void clearPaths();

	template<class T>
	T* loadShared( std::string fileName )
	{
		std::string fp = filePath(fileName);
		reObject* obj;
		if (sharedObjects.find(fp) == sharedObjects.end())
		{
			obj = new T;
			loadObject(obj, fp);
			sharedObjects[fp] = reObjectEntry(obj);
		} else
		{
			obj = sharedObjects[fp].object;
		}
		sharedObjects[fp].retainCount++;
		return (T*)obj;
	}
	void releaseShared(reObject* object);
};

#endif // reAssetLoader_h__