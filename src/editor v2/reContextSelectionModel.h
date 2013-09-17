#ifndef reContextSelectionModel_h__
#define reContextSelectionModel_h__

#include "std.h"

#include "reObject.h"
#include "reCollection.h"
#include "reTypes.h"
#include "reNode.h"

struct reSelectionState
{
	QList<reIntVector> selected;
	reIntVector current;
};

class reContextSelectionModel: public QItemSelectionModel
{
	Q_OBJECT
private:
	bool updating;
	reSelectionState current;
public:
	QUndoStack* commandStack;
	reContextSelectionModel(QAbstractItemModel* model);
	
	reSelectionState state();
	void state(reSelectionState);

	virtual void select ( const QModelIndex & index, QItemSelectionModel::SelectionFlags command );
	virtual void select ( const QItemSelection & selection, QItemSelectionModel::SelectionFlags command );
	void setCurrentIndex ( const QModelIndex & index, QItemSelectionModel::SelectionFlags command );
	reObjectList selectedObjects();
	void selectObjects( reNodeVector& nodes );

public slots:
	void structureAboutToBeChanged();
	void stateChanged();
	reNodeVector selectedNodes();
};

class reSelectionUpdateCommand: public QUndoCommand
{
private:
	reContextSelectionModel* model;
	reSelectionState previous, next;
public:
	reSelectionUpdateCommand(reContextSelectionModel* model, reSelectionState previous, reSelectionState next);
	void undo();
	void redo();
};

#endif // reContextSelectionModel_h__