#ifndef RECOLLECTIONMODEL_H
#define RECOLLECTIONMODEL_H

#include "reCollection.h"

#include <QMimeData>
#include <QAbstractItemModel>

using namespace std;

class reContextModel: public QAbstractItemModel
{
public:
	reCollection* root;
	reContextModel();
	void collection(reCollection* collection);
	reObject* object(const QModelIndex& idx) const;
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(reObject* obj) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	
	void remove( reObjectList& objects );
	//virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
	//virtual QStringList mimeTypes() const;
	//bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
	//Qt::DropActions supportedDragActions () const;
	//Qt::DropActions supportedDropActions () const;
	//Qt::ItemFlags flags ( const QModelIndex & index ) const;
};

#endif // RECOLLECTIONMODEL_H
