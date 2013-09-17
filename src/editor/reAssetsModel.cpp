#include <gl/glew.h>

#include "reAssetsModel.h"

#include <QStandardItemModel>
#include <QFormLayout>
#include <qfilesystemmodel>

#include <iostream>
#include <QDir>
#include <QDirIterator>

#include "reNodeAsset.h"
#include "reNode.h"
#include "reRadial.h"
#include "reEditorAssetLoader.h"

#include <qfilesystemmodel.h>

Qt::DropActions reAssetModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

int reAssetModel::rowCount( const QModelIndex & parent /*= QModelIndex() */ ) const
{
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		return root->children.size();
	return 
		((reAsset*)parent.internalPointer())->children.size();
}

reAssetModel::reAssetModel(QObject* parent):
QAbstractItemModel(parent)
{
	root = new reFileAsset(NULL);
	//rootItem = new reFileAsset(root);
	//rootItem->parent = root;
	//root->children->add(rootItem);
}

reAssetModel::~reAssetModel()
{

}

int reAssetModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	if (!parent.isValid())
		return root->columnCount();
	reAsset* item = (reAsset*)parent.internalPointer();
	return item->columnCount();
}

QModelIndex reAssetModel::parent( const QModelIndex &index ) const
{
	if (!index.isValid())
		return QModelIndex();
	reAsset* item = (reAsset*)index.internalPointer();
	reAsset* parent = item->parent;
	return (parent == root || !parent) ? QModelIndex(): createIndex(parent->row(), 0, parent);
}

QModelIndex reAssetModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	//if (!hasIndex(row, column))
		//return QModelIndex();
	reAsset* parentItem; 
	if (!parent.isValid())
	{
		parentItem = root;
	} else 
	{
		parentItem = (reAsset*)parent.internalPointer();
	}
	reAsset* child = parentItem->getChildAt(row);	
	return child ? createIndex(row, column, child) : QModelIndex();
}

QModelIndex reAssetModel::index( reAsset* item  ) const
{
	reAssetVector itemList;
	while (item->parent)
	{
		itemList.push_back(item);
		item = item->parent;
	}
	QModelIndex idx = QModelIndex();
	for (int i=itemList.size()-1; i >= 0; i--)
	{
		idx = index(itemList[i]->row(), 0, idx);
	}
	if (!idx.isValid())
	{
		qDebug() << "fuck";
	}
	return idx;
}

QModelIndex reAssetModel::index( QString path ) const
{	
	reAsset* item = root->nodeByPath(path);
	return index(item);
}

QVariant reAssetModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return root->data(section);
	return QVariant();
}

Qt::ItemFlags reAssetModel::flags( const QModelIndex &index ) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

QVariant reAssetModel::data( const QModelIndex &index, int role ) const
{
	if (!index.isValid())
		return QVariant();
	reAsset* item = (reAsset*)index.internalPointer();
	switch (role)
	{
	case Qt::DisplayRole:
		return item->data(index.column());
	case Qt::DecorationRole:
		if (item->icon.isNull()) 
		{
			reFileAsset* fileItem = dynamic_cast<reFileAsset*>(item);
			if (fileItem)
			{
				fileItem->icon = iconProvider.icon(fileItem->info);
			}			
		}
		return item->icon;
	default:
		return QVariant();
	}
}

bool reAssetModel::setData(const QModelIndex &index, const QVariant &value, int role /* = Qt::EditRole */)
{
	return false;
}

bool reAssetModel::removeRows(int row, int count, const QModelIndex &parent /* = QModelIndex */)
{
	reAsset* item = node(parent); 
	beginRemoveRows(parent, row, row+count-1);	
	for (reAssetVector::iterator it = item->children.begin()+row; it != item->children.begin()+row+count; it++)
	{
		//delete *it;
	}
	//item->children.erase(item->children.begin()+row, item->children.begin()+row+count);
	/*for (int i = row+count; i >= row; i--)
	{
		//delete (item->children[i]);
		item->children.erase(i);
	}
	*/
	endRemoveRows();
	return true;
}

bool reAssetModel::insertRows(int row, int count, const QModelIndex &parent /* = QModelIndex */)
{
	beginInsertRows(parent, row, row+count-1);
	endInsertRows();
	return true;
}

/*
bool reFileModel::hasChildren( const QModelIndex &parent ) const
{
	return !parent.isValid() || ((reAssetItem*)parent.internalPointer())->info.isDir();
}
*/

reAsset* reAssetModel::node(QModelIndex index) const
{
	if (index.isValid())
	{
		return ((reAsset*)index.internalPointer());
	}
	return NULL;
}

/*
void reFileModel::reload(reAssetItem* node)
{
	
}
*/

void reAssetModel::refresh(reAsset* item) 
{
	QModelIndex id = index(item);
	//if (!id.isValid()) return;
	if (item->children.size())
	{
		beginRemoveRows(id, 0, item->children.size()-1);
		endRemoveRows();
	}
	item->load();
	if (item->children.size())
	{
		beginInsertRows(id, 0, item->children.size()-1);
		endInsertRows();
	}
}

void reAssetModel::refresh(QString path) 
{
	// qDebug() << path;
	reAsset* item = root->nodeByPath(path);
	refresh(item);
}

bool reAssetModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
									int row, int column, const QModelIndex &parent)
{
	if (!parent.isValid())
		return false;
	reFileAsset *target = dynamic_cast<reFileAsset*>(node(parent));
	if (!target || !target->info.isDir())
		return false;

	QString json = data->data("text/json");
	if (json.length())
	{		
		Json::Reader reader;	
		reVar root, items;
		reader.parse(json.toStdString(), items);
		for (int i=0; i<items.size(); i++)
		{
			root = items[i];
			reNode* anode = (reNode*)reRadial::shared()->assetLoader()->loadObject(root);

			QString to = target->path() + QDir::separator() + anode->name().c_str() + ".json";
			if (root.isMember("id"))
			{
				reNode* source = reRadial::shared()->getNode(root["id"].asInt());
				anode->transform(source->worldTransform());
			}
			anode->save(to.toStdString());
		}
		// reNodeAsset* asset = new reNodeAsset(parent);
		// asset->node(enode, true);
		// target->children->add(asset);
		refresh(target);
		return true;
	}

	bool success = true;	
	QString to = target->path();	

	QList<QUrl> urls = data->urls();
	QList<QUrl>::const_iterator it = urls.constBegin();
	QSet<QString> paths; 

	action = Qt::MoveAction;

	switch (action) {
	case Qt::CopyAction:
		for (; it != urls.constEnd(); ++it) {
			QString path = (*it).toLocalFile();
			success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
		}
		break;
	case Qt::LinkAction:
		for (; it != urls.constEnd(); ++it) {
			QString path = (*it).toLocalFile();
			success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
		}
		break;
	case Qt::MoveAction:
		for (; it != urls.constEnd(); ++it) {
			QString path = (*it).toLocalFile();
			success = QFile::rename(path, to + QDir::separator() + QFileInfo(path).fileName()) && success;
			QFileInfo info = QFileInfo(path);
			paths.insert(info.absolutePath());
			// node(index(info.absolutePath()))->load();
			//parent = index(_path);
		}
		break;
	default:
		return false;
	}

	paths.insert(to);

	QSet<QString> nodesToLoad;
	foreach(QString s1, paths) 
	{
		bool found = false;
		foreach(QString s2, paths) 
		{
			if (s1!=s2 && s1.startsWith(s2))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			nodesToLoad.insert(s1);
		}
	}
	foreach(QString s, nodesToLoad) 
	{
		//node(index(s))->load();
		qDebug() << "refreshing " << s;
		refresh(node(index(s)));
	}	
	return success;
}

QStringList reAssetModel::mimeTypes() const
{
	QStringList ret;
	ret.append("text/uri-list");
    ret.append("text/json");
	return ret;
}

QMimeData *reAssetModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QList<QModelIndex>::const_iterator it = indexes.begin();
	
    for (; it != indexes.end(); ++it)
		if ((*it).column() == 0) {
			reFileAsset *fe = dynamic_cast<reFileAsset*>(node(*it));
			if (fe)
			{
				urls << QUrl::fromLocalFile(fe->path());
				continue;
			}            
			reNodeAsset *ne = dynamic_cast<reNodeAsset*>(node(*it));
			if (ne)
			{
				urls << QUrl::fromLocalFile(QString(ne->nodeFilePath().c_str()));
				continue;
			}
		}
	
    QMimeData *data = new QMimeData();
    data->setUrls(urls);
    return data;
}
