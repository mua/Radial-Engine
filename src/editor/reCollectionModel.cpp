#include "reCollectionModel.h"

#include <QStringList>

reContextModel::reContextModel()
{
	root = 0;
}

void reContextModel::collection( reCollection* collection )
{
	root = collection;
}

reObject* reContextModel::object( const QModelIndex& idx ) const
{
	return idx.isValid() ? (reObject*)idx.internalPointer() : root;
}

QModelIndex reContextModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return createIndex(row, column, ((reCollection*)object(parent))->at(row));
}

QModelIndex reContextModel::index( reObject* obj ) const
{
	return obj != root ? index(obj->super->indexOf(obj), 0, index(obj->super)) : QModelIndex();
}

QModelIndex reContextModel::parent( const QModelIndex &child ) const
{
	return index(object(child)->super);
}

int reContextModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return ((reCollection*)object(parent))->count();
}

int reContextModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 1;
}

QVariant reContextModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	if (role == Qt::DisplayRole)
	{
		return object(index)->className().c_str();
	}
	return QVariant();
}
/*
QMimeData* reCollectionModel::mimeData( const QModelIndexList &indexes ) const
{
	QMimeData* mime = new QMimeData();
	Json::Value list(Json::arrayValue);
	for (int i=0; i<indexes.size(); i++)
	{
		list.append(object(indexes.at(i))->toJson());
	}
	mime->setData("text/json", list.toStyledString().c_str());
	std::cout << list;
	std::cout.flush();
	return mime;
}

QStringList reCollectionModel::mimeTypes() const
{
	QStringList list;
	list.append("text/json");
	return list;
}

bool reCollectionModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
	QString json = data->data("text/json");
	if (json.isEmpty()) return false;
	Json::Value val(Json::arrayValue); Json::Reader reader;
	reader.parse(json.toStdString(), val);
	for (unsigned int i=0; i<val.size(); i++)
	{
		reObject *obj = reObject::toJson(val[i]);
		if (row > -1)
			((reCollection*)object(parent))->insert(obj, row+i);
		else
			((reCollection*)object(parent))->add(obj);
	}
	beginInsertRows(parent, row+1, row + val.size());
	endInsertRows();
	return true;
}

Qt::DropActions reCollectionModel::supportedDragActions() const
{
	return Qt::CopyAction | Qt::MoveAction; // | Qt::LinkAction;
}

Qt::DropActions reCollectionModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction; // | Qt::LinkAction;
}

Qt::ItemFlags reCollectionModel::flags( const QModelIndex & index ) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
	if (dynamic_cast<reNodeCollection*>(object(index)) || dynamic_cast<reRenderableCollection*>(object(index)))
		return Qt::ItemIsDropEnabled | defaultFlags;
	else
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}
*/