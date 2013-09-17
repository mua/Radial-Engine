#include "std.h"
#include "reContextSelectionModel.h"
#include "reContextModel.h"
#include "reNode.h"

reContextSelectionModel::reContextSelectionModel( QAbstractItemModel* model ):
	QItemSelectionModel(model)
{
	updating = false;
	connect(model, SIGNAL(structureAboutToBeChanged()), SLOT(structureAboutToBeChanged()));
	connect(model, SIGNAL(stateChanged()), SLOT(stateChanged()));
}

void reContextSelectionModel::select( const QModelIndex & index, QItemSelectionModel::SelectionFlags command )
{
	QItemSelectionModel::select(index, command);
}

void reContextSelectionModel::select( const QItemSelection & selection, QItemSelectionModel::SelectionFlags command )
{
	reSelectionState prev;
	if (!updating)
		prev = state();
	QItemSelectionModel::select(selection, command);
	if (!updating)
	{
		updating = true;
		commandStack->push(new reSelectionUpdateCommand(this, prev, state()));
		updating = false;
	}
}

void reContextSelectionModel::setCurrentIndex( const QModelIndex & index, QItemSelectionModel::SelectionFlags command )
{
	reSelectionState prev = state();
	QItemSelectionModel::setCurrentIndex(index, command);
	if (!updating)
	{
		updating = true;
		commandStack->push(new reSelectionUpdateCommand(this, prev, state()));
		updating = false;
	}
}

reSelectionState reContextSelectionModel::state()
{	
	reSelectionState state;
	foreach (QModelIndex idx, selectedRows())
	{
		state.selected.append(((reContextModel*)model())->path(idx));
	}
	//state.current = ((reContextModel*)model())->path(currentIndex());
	return state;
}

void reContextSelectionModel::state( reSelectionState state )
{
	if (updating) return;
	updating = true;
	QItemSelection selection;
	foreach(reIntVector path, state.selected)
	{
		QModelIndex sidx = ((reContextModel*)model())->index(path);
		selection.append(QItemSelectionRange(sidx, sidx));
	}
	select(selection, QItemSelectionModel::Clear | QItemSelectionModel::Select);
	//setCurrentIndex(((reContextModel*)model())->index(state.current), QItemSelectionModel::NoUpdate);
	updating = false;
}

void reContextSelectionModel::structureAboutToBeChanged()
{	
	if (!updating)
	{
		current = state();
		updating = true;
	}	
}

void reContextSelectionModel::stateChanged()
{
	if (updating)
	{
		updating = false;
		commandStack->push(new reSelectionUpdateCommand(this, current, state()));
	}
}

reObjectList reContextSelectionModel::selectedObjects()
{
	reObjectList selected;
	foreach (QModelIndex idx, selectedRows())
	{
		selected.push_back(((reContextModel*)model())->object(idx));
	}
	return selected;
}

void reContextSelectionModel::selectObjects( reNodeVector& nodes )
{
	QItemSelection selection;
	for(reNodeVector::iterator it = nodes.begin(); it != nodes.end(); it++)
	{
		QModelIndex sidx = ((reContextModel*)model())->index(*it);
		selection.append(QItemSelectionRange(sidx, sidx));
	}
	select(selection, QItemSelectionModel::Clear | QItemSelectionModel::Select);
}

reNodeVector reContextSelectionModel::selectedNodes()
{
	reNodeVector selected;
	foreach (QModelIndex idx, selectedRows())
	{
		if (reNode* node = dynamic_cast<reNode*>(((reContextModel*)model())->object(idx)))
			selected.push_back(node);
	}
	return selected;
}

//////////////////////////////////////////////////////////////////////////

reSelectionUpdateCommand::reSelectionUpdateCommand( reContextSelectionModel* model, reSelectionState previous, reSelectionState next )
{
	this->model = model;
	this->previous = previous;
	this->next = next;
}

void reSelectionUpdateCommand::undo()
{
	model->state(previous);
}

void reSelectionUpdateCommand::redo()
{
	model->state(next);
}
