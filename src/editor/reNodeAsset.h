#ifndef reNodeAsset_h__
#define reNodeAsset_h__

#include "reAsset.h"

#include <string>
#include <json/json.h>
#include <QFileSystemModel>

using namespace std;

class reNode;

class reNodeAsset:
	public reAsset
{
private:
	reNode* _node;
	string _nodeFilePath;
public:
	reNodeAsset(reAsset* parent);
	virtual QVariant data(int column) const;
	virtual Json::Value getJson();
	virtual void loadJson( Json::Value& val );
	//virtual void saveNode();
	reNodeAsset* childByName(string& name);
	void loadNode();

	reNode* node() const;
	void node(reNode* val, bool recursive = false);
	std::string nodeFilePath() const;
	void nodeFilePath(std::string val);
	std::string name() const;
	void addChildNodeAsset(reNodeAsset* child);
	void removeChildNodeAsset(reNodeAsset* child);
	virtual QWidget* createEditor( QWidget *parent );
};
#endif // reNodeAsset_h__
