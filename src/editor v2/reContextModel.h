#ifndef RECOLLECTIONMODEL_H
#define RECOLLECTIONMODEL_H

#include "std.h"

#include "reCollection.h"
#include "reTypes.h"

#include <QMimeData>
#include <QAbstractItemModel>

#include <map>
using namespace std;

typedef map<reObject*, reVar> reStateMap;

class reContextModel: public QAbstractItemModel
{
	Q_OBJECT
private:
	reCollection *_root;
public:
	QUndoStack* commandStack;

	reCollection* base;
	reContextModel();
	void collection(reCollection* collection);
	reObject* object(const QModelIndex& idx) const;
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(reObject* obj) const;	
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
	virtual QStringList mimeTypes() const;
	bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
	Qt::DropActions supportedDragActions () const;
	Qt::DropActions supportedDropActions () const;
	Qt::ItemFlags flags ( const QModelIndex & index ) const;

	void insert(reObject* object, reCollection* parent, int row = -1);
	void remove(reObject* object);
	void remove( reObjectList& objects );
	void sync( reCollection* collection, reVar prev, reVar next );

	reStateMap* stateMap;
	void beginUpdates(reObjectList& objects);
	void beginUpdates(reObject* object);
	void cancelUpdates();
	void endUpdates();

	reIntVector path(QModelIndex _idx);
	QModelIndex index( reIntVector& path ) const;

	reCollection * root() const;
	void root(reCollection * val);
signals:
	void structureAboutToBeChanged();
	void stateChanged();
};

class reContextUpdateCommand: public QUndoCommand
{
public:
	reStateMap *previous, *next;
	bool done;
	reContextModel* model;
	reContextUpdateCommand(reContextModel* model, reStateMap* previous, reStateMap* next);
	~reContextUpdateCommand();
	void undo();
	void redo();
};

#endif // RECOLLECTIONMODEL_H
