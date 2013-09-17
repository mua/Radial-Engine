#include "reAsset.h"

#include "reFbxAsset.h"
#include "reNodeAsset.h"
#include "reFileAsset.h"

reAsset::reAsset( reAsset* _parent )
{
	parent = _parent;
}

reAsset::~reAsset()
{

}

QWidget * reAsset::createEditor( QWidget *parent )
{
	return NULL;
}

void reAsset::clearChildren()
{
	for (reAssetVector::iterator it=children.begin(); it != children.end(); it++) {
		delete (*it);
	}
	children.clear();
}

void reAsset::load()
{

}; 

QVariant reAsset::data( int column ) const
{
	return "Asset";
}

reAsset* reAsset::getChildAt( int row )
{
	if (row>=0 && (row < (int)children.size()))
	{
		return children[row];
	}	
	return NULL;
}

int reAsset::row()
{
	reAssetVector::iterator it = 
		std::find(parent->children.begin(), parent->children.end(), this);
	if (it<parent->children.end()) return (it-parent->children.begin());
	return -1;
}

int reAsset::columnCount() const
{
	return 1;
}

reAsset* reAsset::createChild(const QFileInfo& path, reAsset* parent) 
{
	if (path.completeSuffix().toLower() == "fbx") {
		return new reFBXAsset(parent);
	}
	return new reFileAsset(parent);
}

reAsset* reAsset::createChild(QString name, reAsset* parent) 
{
	if (name == "reFBXAsset") {
		return new reFBXAsset(parent);
	}
	if (name == "reNodeAsset") {
		return new reNodeAsset(parent);
	}
	return new reFileAsset(parent);
}

Json::Value reAsset::getJson()
{
	return Json::Value();
}

void reAsset::loadJson( Json::Value& val )
{
	
}

void reAsset::addChild( reAsset* child )
{
	children.push_back(child);
	child->parent = this;
}

void reAsset::removeChild( reAsset* child )
{
	reAssetVector::iterator it = std::find(children.begin(), children.end(), child);
	if (it != children.end())
	{
		children.erase(it);
	}
}