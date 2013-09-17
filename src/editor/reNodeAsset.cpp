#include "reNodeAsset.h"

#include "reNode.h"

#include <QString>
#include <QDir>

#include <sstream>

#include <json/json.h>

#include <QtCore/QDate>
#include <QtCore/QLocale>
#include <QBoxLayout>
#include <QtCore/QDate>
#include <QtCore/QLocale>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QDebug>
#include <QDir>
#include <QDirIterator>

reNodeAsset::reNodeAsset( reAsset* parent ):
	reAsset(parent)
{
	_nodeFilePath  = "";
}

QVariant reNodeAsset::data( int column ) const
{
	return QString(name().c_str());
}

Json::Value reNodeAsset::getJson()
{
	Json::Value val = reAsset::getJson();
	val["name"] = name().c_str();
	val["nodeFile"] = nodeFilePath().c_str();
	return val;
}

void reNodeAsset::loadJson( Json::Value& val )
{
	reAsset::loadJson(val);
	nodeFilePath(val["nodeFile"].asString());
}
/*
void reNodeAsset::saveNode()
{
	QString path = (parent->dataDir() + QDir::separator() + name().c_str() + ".json");
	node()->save(path.toStdString());
	nodeFilePath(path.toStdString());
}
*/
/*
void reNodeAsset::loadNode()
{
	if (!nodeFilePath().empty())
	{
		reNode* anode = new reNode();
		anode->load(nodeFilePath());
		node(anode, true);
	}
}
*/
reNode* reNodeAsset::node() const
{
	return _node;
}

void reNodeAsset::addChildNodeAsset(reNodeAsset* child)
{
	string base = child->node()->name();
	stringstream s;
	int i = 1;
	while (childByName(child->node()->name())) {
		s.clear();
		s << base;  s << "_"; s << i;
		child->node()->name(s.str());
		i++;
	}
	child->parent = this;
	children.push_back(child);
}

void reNodeAsset::node( reNode* val, bool recursive )
{	
	_node = val;
	children.clear();
	if (recursive)
	{
		for (int i=0; i<val->children->count(); i++)
		{
			reNodeAsset *childAsset = new reNodeAsset(this);
			childAsset->node((reNode*)val->children->at(i), true);
			children.push_back(childAsset);
		}
	}
}

std::string reNodeAsset::nodeFilePath() const
{
	return _nodeFilePath;
}

void reNodeAsset::nodeFilePath( std::string val )
{
	_nodeFilePath = val;
}

std::string reNodeAsset::name() const
{
	return node() ? node()->name(): "";
}

reNodeAsset* reNodeAsset::childByName( string& name )
{
	for (int i=0; i<children.size(); i++)
	{
		reNodeAsset* childNode = dynamic_cast<reNodeAsset*>(children[i]);
		if (childNode && childNode->name() == name)
			return childNode;
	}
	return NULL;
}

QWidget* reNodeAsset::createEditor( QWidget *parent )
{
	editor = new QWidget(parent);
	return editor;
}