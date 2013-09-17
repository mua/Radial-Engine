#include "reContext.h"

#include <QMimeData>
#include <QDebug>
#include <QStringList>
#include <QUrl>

#include "reNode.h"
#include "json/json.h"

#include <algorithm>
#include <map>

#include "reRadial.h"
#include "reAssetLoader.h"
#include "reRefNode.h"

reSceneModel::reSceneModel(QObject* parent):
	QAbstractItemModel(parent)
{
	base = 0;
}

reSceneModel::~reSceneModel()
{

}

//////////////////////////////////////////////////////////////////////////

QModelIndex reSceneModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	reCollection *pn = parent.isValid() ? (reCollection*)object(parent) : base;
	return createIndex(row, column, pn->at(row));
}

QModelIndex reSceneModel::index( reObject* object ) const
{
	reCollection* pn = object->super;
	if (pn)
	{
		int pos = pn->indexOf(object);
		if (pos != -1)
		{
			QModelIndex idx = index(object->super);
			return index(pos, 0, idx);
		}
	}
	return QModelIndex();
}

QModelIndex reSceneModel::parent( const QModelIndex &child ) const
{
	if (!child.isValid()) return QModelIndex();

	if (reObject *n = object(child))
	{
		return n->super ? index(n->super) : QModelIndex();
	}
	return QModelIndex();
}

int reSceneModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	if (parent.column() > 0) return 0;
	if (!parent.isValid())
		return base->children->count();
	else 
		return ((reCollection*)object(parent))->count();
}

int reSceneModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 1;
}

QVariant reSceneModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	if (!index.isValid()) return QVariant();
	reObject* item = object(index);
	switch (role)
	{
	case Qt::DisplayRole:
		return item->name().size() ? item->name().c_str() : item->className().c_str();
	case Qt::DecorationRole:
	default:
		return QVariant();
	}
}

Qt::ItemFlags reSceneModel::flags( const QModelIndex &index ) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

Qt::DropActions reSceneModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

//////////////////////////////////////////////////////////////////////////

reNode* reSceneModel::root() const
{
	return _root;
}

void reSceneModel::root( reNode* val )
{	
	if (base)
	{
		base->children->remove(_root);
		delete base;
	}
	_root = val;
	base = new reNode;
	base->children->add(val);
}

reObject* reSceneModel::object( const QModelIndex &idx ) const
{
	return idx.isValid() ? (reNode*)idx.internalPointer() : 0;
}

bool reSceneModel::insertRows( int row, int count, const QModelIndex &parent /*= QModelIndex()*/ )
{
	beginInsertRows(parent, row, row+count-1);
	endInsertRows();
	return true;
}

bool reSceneModel::removeRows( int row, int count, const QModelIndex &parent /*= QModelIndex()*/ )
{
	beginRemoveRows(parent, row, row+count-1);
	endRemoveRows();
	return true;
}

QStringList reSceneModel::mimeTypes() const
{
	QStringList ret;
	ret.append("text/uri-list");
	ret.append("text/json");
	return ret;
}

QMimeData* reSceneModel::mimeData(const QModelIndexList &indexes) const
{
	QList<QModelIndex>::const_iterator it = indexes.begin();

	qDebug() << indexes.count();

	QMimeData *data = new QMimeData();

	reVar items;
	for (; it != indexes.end(); ++it)
	{ 
		reObject *ne = object(*it);
		if (ne)
		{
			reVar json;
			ne->toJson(json);
			//json["id"] = ne->id;
			items.append(json);
			//qDebug() << Json.asJson().toStyledString().c_str();				
		}
	}
	data->setData("text/json", items.asJson(),toStyledString().c_str());
	return data;
}

bool reSceneModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &_parent )
{
	if (!_parent.isValid()) return false;

	reObject *obj = object(_parent);
	reNodeCollection* nc = dynamic_cast<reNodeCollection*>(obj);
	reNode* parentNode = nc ? ((reNode*)nc->super): 0;
	
	Json::Reader reader;	
	Json::Value root, items;	
	QString json = data->data("text/json");
	if (json.size())
	{	
		reNodeVector remove;
		context->commandStack.beginMacro("move");
		context->selectionModel->clear();
		reader.parse(json.toStdString(), items);
		for (int i=0; i<items.size(); i++)
		{
			root = items[i];			
			reNode* anode = (reNode*)reRadial::shared()->assetLoader()->loadObject(root);
			anode->transform(reTransform(parentNode->worldTransform().inverseMatrix() * anode->transform().matrix));			
			if (root.isMember("id")) 
			{
				reNode* snode = reRadial::shared()->getNode(root["id"].asInt());
				if (snode->isChildNode(parentNode))
				{
					continue;
				}
				remove.push_back(snode);
			}
			context->addObject(parentNode, anode, row);
		}
		context->commandStack.beginMacro("delete");
		for (int i=0; i<remove.size(); i++)
		{
			context->deleteObject(remove[i]);
		}
		context->commandStack.endMacro();
		context->commandStack.endMacro();
		return true;
	} 
	
	if (data->urls().size())
	{
		context->commandStack.beginMacro("move");
		foreach(QUrl url,data->urls())
		{
			reRefNode *refNode = new reRefNode();
			refNode->loadNodeFile(url.toLocalFile().toStdString());
			refNode->transform(reTransform(obj->worldTransform().inverseMatrix() * refNode->transform().matrix));
			context->addObject(obj, refNode, row);
		}
		context->commandStack.endMacro();
	}
	return true;
}

bool reSceneModel::setData( const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/ )
{
	switch (role)
	{
	case Qt::EditRole:
		reNodeVector v;
		v.push_back(object(index));
		context->beginUpdatingObjects(v);
		object(index)->name(value.toString().toStdString());
		context->endUpdatingObjects();
		return true;
	}
	return false;
}

void reSceneModel::emitNodeUpdate( reNode* anode )
{
	emit dataChanged(index(anode), index(anode));
	reNodeVector vec; vec.push_back(anode);
	context->emitObjectsUpdated(vec);
}

//////////////////////////////////////////////////////////////////////////

reContext::reContext( reNode* node )
{
	root = node;
	model = new reSceneModel();
	model->root(root);
	model->context = this;
	selectionModel = new reSceneSelectionModel(model);
	connect(selectionModel, 
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	commandRunning = false;
}

reContext::~reContext()
{
	delete model;
}

void reContext::selectObjects( reObjectList& objects )
{
	//selectionModel->clear();
	QItemSelection selection;
	for (reObjectList::iterator it=objects.begin(); it!=objects.end(); it++)
	{		
		selection.append(QItemSelectionRange(model->index(*it)));		
	}
	selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Clear);
}

reObjectList reContext::selectedObjects()
{
	reObjectList vec;
	foreach (QModelIndex idx, selectionModel->selectedIndexes())
	{
		vec.push_back(model->object(idx));
	}
	return vec;
}

void reContext::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
	if (!commandRunning)
	{
		commandStack.push(new reSelectCommand(this, selectedObjects()));
	}	
	currentSelectedObjects = selectedObjects();
}

void reContext::deleteObject( reObject* object )
{	
	reDeleteCommand* cmd = new reDeleteCommand(this, object);
	commandStack.push(cmd);
}

void reContext::deleteObjects( reObjectList& objects )
{
	commandStack.beginMacro("delete");
	selectionModel->clear();
	reDeleteCommand* cmd = 0;
	for (reObjectList::iterator it = objects.begin(); it < objects.end(); it++)
	{
		reDeleteCommand* acmd = new reDeleteCommand(this, *it);
		commandStack.push(acmd);
	}
	commandStack.endMacro();
}

void reContext::addObject( reCollection* parent, reObject* child, int row )
{
	reAddCommand* cmd = new reAddCommand(this, parent, child, row);
	commandStack.push(cmd);
}

void reContext::beginUpdatingObjects( reObjectList& objects )
{	
	updatingStates.clear();
	for (reObjectList::iterator it=objects.begin(); it!=objects.end(); it++)
	{		
		updatingStates.push_back((*it)->state());
	}
	updatingObjects = objects;
}

void reContext::endUpdatingObjects()
{	
	bool dirty = false;
	typedef std::map<reObject*, reObjectState*> reStateMap; 
	reStateMap stateMap;
	for (int i=0; i<updatingObjects.size(); i++)
	{
		reNodeState *state = updatingObjects[i]->state();
		if (!(state->isEqual(updatingStates[i])))
		{
			stateMap[updatingObjects[i]] = updatingStates[i];
		} 
		else 
		{
			delete state;
		}
	}
	if (stateMap.size())
	{
		commandStack.beginMacro("modify");		
		for (reStateMap::iterator it = stateMap.begin(); it != stateMap.end(); it++)
		{
			commandStack.push(new reNodeUpdateCommand(this, it->first, it->second, it->first->state()));
		}
		commandStack.endMacro();
	}
	emitObjectsUpdated(updatingObjects);
}

void reContext::emitObjectsUpdated( reObjectList& objects )
{
	emit objectsUpdated(objects);
}
//////////////////////////////////////////////////////////////////////////

reContextCommand::reContextCommand( reContext* ctx )
{
	this->ctx = ctx;
}

//////////////////////////////////////////////////////////////////////////

reSelectCommand::reSelectCommand( reContext* ctx, reNodeVector& nodes ):
	reContextCommand(ctx)
{
	this->objects = nodes;
	oldObjects = ctx->currentSelectedObjects;
}

void reSelectCommand::undo()
{
	ctx->commandRunning = true;
	ctx->selectObjects(oldObjects);
	ctx->commandRunning = false;
}

void reSelectCommand::redo()
{
	if (objects != ctx->selectedObjects())
	{
		ctx->commandRunning = true;
		ctx->selectObjects(objects);
		ctx->commandRunning = false;
	}
}

//////////////////////////////////////////////////////////////////////////

reDeleteCommand::reDeleteCommand( reContext *ctx, reObject* object ):
	reContextCommand(ctx)
{
	deletedObject = object;
	parentObject = object->super;
}

void reDeleteCommand::undo()
{
	if (parentObject)
	{
		parentObject->add(deletedObject);
		ctx->model->insertRow(row, ctx->model->index(parentObject));
	}
}

void reDeleteCommand::redo()
{
	if (parentObject)
	{
		row = parentObject->indexOf(deletedObject);
		ctx->model->removeRow(row, ctx->model->index(parentObject));
		parentObject->remove(deletedObject);	
	}
}

reDeleteCommand::~reDeleteCommand()
{
	//delete deletedNode;
}

//////////////////////////////////////////////////////////////////////////

reAddCommand::reAddCommand( reContext *ctx, reCollection* parent, reObject* child, int row ):
	reContextCommand(ctx)
{
	parentObject = parent;
	addedObject = child;
	this->row = row;
}

void reAddCommand::undo()
{
	ctx->model->removeRow(row, ctx->model->index(parentObject));
	parentObject->remove(addedObject);
}

void reAddCommand::redo()
{
	parentObject->insert(addedObject, row);
	row = parentObject->indexOf(addedObject);
	ctx->model->insertRow(row, ctx->model->index(parentObject));
}

reAddCommand::~reAddCommand()
{

}

//////////////////////////////////////////////////////////////////////////

reNodeUpdateCommand::reNodeUpdateCommand( reContext *ctx, reObject* node, reObjectState* begin, reObjectState* end )
	:reContextCommand(ctx)
{
	this->object = node;
	this->begin = begin;
	this->end = end;
	qDebug() << "new state command";
}

void reNodeUpdateCommand::undo()
{
	object->state(begin);
	ctx->model->emitObjectUpdate(object);
}

void reNodeUpdateCommand::redo()
{
	object->state(end);
	ctx->model->emitObjectUpdate(object);
}

reNodeUpdateCommand::~reNodeUpdateCommand()
{

}