#include "reAssetLoader.h"
#include "reMaterial.h"

#include "reNode.h"
#include "reRefNode.h"
#include "reMesh.h"
#include "reUtilities.h"
#include "reAnimationSet.h"
#include "reBone.h"
#include "rePTerrain.h"
#include "rePhysics.h"
#include "reBody.h"
#include "reCamera.h"
#include "reVehicle.h"
#include "reLight.h"
#include "reMaterial.h"
#include "reMeshRenderable.h"
#include "reEnemy.h"
#include "reScatter.h"
#include "reRunner.h"
#include "reBuilding.h"

#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

#define reREGISTER_CLASS(X) classes[#X] = new reClassEntry<X>();

reAssetLoader::reAssetLoader()
{
	reREGISTER_CLASS(reCollection);
	reREGISTER_CLASS(reNode);
	reREGISTER_CLASS(reRefNode);
	reREGISTER_CLASS(reMesh);
	reREGISTER_CLASS(reRenderableCollection);
	reREGISTER_CLASS(reNodeCollection);
	reREGISTER_CLASS(reAnimator);
	reREGISTER_CLASS(reSequence);
	reREGISTER_CLASS(reBone);
	reREGISTER_CLASS(rePTerrain);
	reREGISTER_CLASS(reBone);	
	reREGISTER_CLASS(reBody);
	reREGISTER_CLASS(rePhysics);
	reREGISTER_CLASS(rePointCamera);
	reREGISTER_CLASS(reVehicle);
	reREGISTER_CLASS(reLight);
	reREGISTER_CLASS(reTexture);
	reREGISTER_CLASS(reMeshRenderable);
	reREGISTER_CLASS(reAnimset);
	reREGISTER_CLASS(reEnemy);
	reREGISTER_CLASS(reScatter);
	reREGISTER_CLASS(reRunner);
	reREGISTER_CLASS(reBuilding);
}

void reAssetLoader::loadImage( std::string& fileName, reImage* out)
{

}

reObject* reAssetLoader::loadObject( reVar& var )
{	
	string classname = var["className"].asString();
	reObject* node = classes[classname]->instance();
	if	(node) 
	{
		//node->path(var["path"].asString());
		//var.removeMember("path");
		node->loadJson(var);
	}
	return node;
}

reObject* reAssetLoader::loadObject( std::string& fileName, bool asRef )
{
	reObject* node = NULL;
	ifstream fs(filePath(fileName).c_str(), ios::in);
	if (!fs.fail())
	{
		Json::Reader reader;
		Json::Value root;
		reader.parse(fs, root);
		if (!asRef)
		{
			root.removeMember("path");
			root.removeMember("isReference");
		}		
		node = loadObject(reVar(root));
		fs.close();
	}	
	return node;
}

void reAssetLoader::loadObject( reObject* obj, std::string& fileName )
{
	std::cout << "loading Object:" << fileName << std::endl;
	obj->loadFromFile(fileName);
}

std::string reAssetLoader::loadFile( std::string& fileName )
{
	std::cout << "loading File:" << fileName << std::endl;
	reNode* node = NULL;
	ifstream fs(filePath(fileName).c_str(), ios::in);
	if (!fs.fail())
	{	
		std::stringstream stream;
		stream << fs.rdbuf();
		return stream.str();
	}
	return "";
}

std::string reAssetLoader::filePath( std::string fileName )
{	
	fileName = strReplace(fileName, "\\", "/");
	if (find(fileName.begin(), fileName.end(), ':') != fileName.end())
	{
		return fileName;
	}
	stringstream ss;
	for (reStringList::iterator it=paths.begin(); it != paths.end(); it++)
	{
		ss.str("");
		ss << *it << "/" << fileName;
		if (!ifstream(ss.str()).fail())
		{
			return ss.str();
		}
	}
	return fileName;
}

std::string reAssetLoader::relativePath( std::string fileName )
{
	fileName = strReplace(fileName, "\\", "/");
	for (reStringList::iterator it=paths.begin(); it != paths.end(); it++)
	{
		if (fileName.find(*it) != string::npos)
		{	
			fileName.erase(fileName.begin(), fileName.begin()+(*it).size());
			return fileName;
		}
	}
	return fileName;
}

void reAssetLoader::clearPaths()
{
	paths.clear();
}

void reAssetLoader::addPath( std::string val )
{
	paths.push_back(strReplace(val, "\\", "/"));
}

void reAssetLoader::releaseShared( reObject* object )
{
	if (!object)
	{
		return;
	}
	reObjectEntry &entry = sharedObjects[object->path()];
	entry.retainCount--;
	if (entry.retainCount == 0)
	{
		reObject *object = entry.object;
		sharedObjects.erase(object->path());
		delete object;
	}
}

reObjectEntry::reObjectEntry()
{

}

reObjectEntry::reObjectEntry( reObject* obj, int retainCount /*= 0*/ )
{
	object = obj;
	this->retainCount = retainCount;
}
