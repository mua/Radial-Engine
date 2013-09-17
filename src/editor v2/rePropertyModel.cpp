#include "std.h"
#include "rePropertyModel.h"
#include "rePropertyEditors.h"

#include <algorithm>

QModelIndex rePropertyModel::index( rePropertyEditor* pset ) const
{
	if (pset->parent())
	{
		int pos = std::find(pset->parent()->subEditors.begin(), pset->parent()->subEditors.end(), pset) - pset->parent()->subEditors.begin();
		return index(pos, 0, index(pset->parent()));
	}
	return QModelIndex();
}

QModelIndex rePropertyModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return root ? createIndex(row, column, editor(parent)->subEditors[row]) : QModelIndex();
}

QModelIndex rePropertyModel::parent( const QModelIndex &child ) const
{
	return child.isValid() ? index(editor(child)->parent()) : QModelIndex();
}

int rePropertyModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	if (parent.column()>0) return 0;
	return root ? editor(parent)->subEditors.size() : 0;
}

int rePropertyModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 2;
}

QVariant rePropertyModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	if (!index.isValid()) return QVariant();
	switch (role)
	{
	case Qt::DisplayRole:
		return !index.column() ? editor(index)->label() : editor(index)->isMultiple() ? "Multiple" : editor(index)->display();
	case Qt::EditRole:
		return index.column() ? editor(index)->read() : QVariant();
	case Qt::SizeHintRole:
		return QSize(20, 20);
	case Qt::FontRole:
		{
			QFont font;
			font.setBold(index.column() == 0);
			return font;
		}
	default:
		return QVariant();
	}
}

bool rePropertyModel::setData( const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/ )
{
	if (!index.column() == 1) return false;
	switch (role)
	{
	case Qt::EditRole:		
		editor(index)->write(value);
		qDebug() << root->pull(0).asJson().toStyledString().c_str();
		emit objectsAboutToBeChanged(objects());
		for (int i = 0; i<objects().size(); i++)
		{
			objects()[i]->loadJson(root->pull(i));
		}
		emit objectsChanged();
	}
	return false;
}

rePropertyEditor* rePropertyModel::editor( const QModelIndex& index ) const
{
	return index.isValid() ? (rePropertyEditor*)index.internalPointer(): root;
}

Qt::ItemFlags rePropertyModel::flags( const QModelIndex &index ) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | (index.isValid() && index.column() && editor(index)->editable() ? Qt::ItemIsEditable : 0);
}

void rePropertyModel::reload()
{
	emit dataChanged(QModelIndex(), QModelIndex());
}

QVariant rePropertyModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (role == Qt::DisplayRole)
	{
		return section ? "Value" : "Property";
	}
	return QVariant();
}

reObjectList rePropertyModel::objects() const
{
	return _objects;
}

void rePropertyModel::objects( reObjectList val )
{
	_objects = val;
	reset();
}

rePropertyModel::rePropertyModel()
{
	root = 0;
}

void rePropertyModel::reset()
{
	delete root;
	if (!objects().size())
	{
		root = 0;
		return;
	}
	//qDebug() << "----------- start";
	root = new reObjectEditor(objects());
	reVarList vars;
	for (unsigned int i =0; i<objects().size(); i++)
	{
		reVar var;
		objects()[i]->toJson(var, reObject::LEAF_PROPERTIES);
		vars.push_back(var);
		//qDebug() << var.asJson().toStyledString().c_str();
	}
	root->push(vars);
	QAbstractItemModel::reset();
}