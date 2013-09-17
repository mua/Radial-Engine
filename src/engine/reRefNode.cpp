#include "reRefNode.h"
#include "reRadial.h"
#include "reAssetLoader.h"

void reRefNode::fileName( std::string val )
{
	_fileName = val;
}

void reRefNode::toJson(reVar& val)
{
	reCollection::toJson(val);
	val.removeMember("children"); 
	val["refPath"] = fileName();
}

void reRefNode::loadJson( reVar& val )
{	
	reCollection::loadJson(val);
	if (val["refPath"].kind != reVar::seNULL)
	{
		loadNodeFile(val["refPath"].asString());
	}
}

void reRefNode::loadNodeFile( std::string fileName )
{
	clear();
	if (fileName.empty())
	{
		return;
	}
	reNode *item = (reNode*)reRadial::shared()->assetLoader()->loadObject(fileName);

	name(item->name());
	add(item);
	/*
	for (reNodeVector::iterator it=item->children.begin(); it != item->children.end(); it++ )
	{
		children->add(*it);
	}
	*/
	this->fileName(fileName);
}

reVar reRefNode::getChildrenJson()
{
	return reVar();
}

std::string reRefNode::className()
{
	return "reRefNode";
}