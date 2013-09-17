#include "std.h"
#include "reContextModel.h"
#include "reMaker.h"

#include "reRadial.h"
#include "reObject.h"
#include "reRefNode.h"
#include "reAssetLoader.h"

reContextModel::reContextModel()
{
	base = 0;	
	stateMap = 0;
}

void reContextModel::collection( reCollection* collection )
{
	base = collection;
	reset();
}

reObject* reContextModel::object( const QModelIndex& idx ) const
{
	return idx.isValid() ? (reObject*)idx.internalPointer() : base;
}

QModelIndex reContextModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	reCollection* coll = (reCollection*)object(parent);
	if (!base || row >= coll->count() || row < 0 || column != 0)
	{
		return QModelIndex();
	}
	return createIndex(row, column, coll->at(row));
}

reIntVector reContextModel::path(QModelIndex _idx)
{
	reIntVector list;
	QModelIndex idx = _idx;
	list.push_back(idx.row());
	while (idx.parent().isValid())
	{
		list.push_back(idx.parent().row());
		idx = idx.parent();
	}
	return list;
}

QModelIndex reContextModel::index( reIntVector& path ) const
{
	reCollection* current = base;
	for (int i=path.size()-1; i > -1; i--)
	{
		if (path[i]<0)
		{
			return QModelIndex();
		}
		current = (reCollection*)current->at(path[i]);
	}
	return index(current);
}

QModelIndex reContextModel::index( reObject* obj ) const
{
	if (obj != base && obj->super && obj->super->indexOf(obj) == -1) return QModelIndex();
	return obj != base ? index(obj->super->indexOf(obj), 0, index(obj->super)) : QModelIndex();
}

QModelIndex reContextModel::parent( const QModelIndex &child ) const
{
	return child.isValid() ? index(object(child)->super) : QModelIndex();
}

int reContextModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	reCollection* obj = dynamic_cast<reCollection*>(object(parent));
	return base && obj ? obj->count() : 0;
}

int reContextModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 1;
}

QVariant reContextModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	if (role == Qt::DisplayRole)
	{
		if (!index.isValid())
		{
			return QVariant();
		}		
		reObject* obj = object(index);
		return obj->name().size()>3 ? obj->name().c_str() : obj->className().c_str();
	}
	if (role == Qt::DecorationRole)
	{
		return reMaker::shared()->getTypeInfo(object(index)->className().c_str())->icon;
	}
	return QVariant();
}

QMimeData* reContextModel::mimeData( const QModelIndexList &indexes ) const
{
	QMimeData* mime = new QMimeData();
	Json::Value list(Json::arrayValue);
	for (unsigned int i =0; i<indexes.size(); i++)
	{
		Json::Value val;
		val["addr"] = (int)object(indexes.at(i));
		list.append(val);
	}
	mime->setData("text/json", list.toStyledString().c_str());
	//std::cout << list;
	//std::cout.flush();
	return mime;
}

QStringList reContextModel::mimeTypes() const
{
	QStringList list;
	list.append("text/json");
	list.append("text/uri-list");
	return list;
}

bool reContextModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
	reCollection* collection = dynamic_cast<reCollection*>(object(parent));
	if (collection && data->hasFormat("text/json"))
	{
		QString json = QString(data->data("text/json"));
		Json::Reader reader;
		Json::Value items;
		reader.parse(json.toStdString(), items);
		commandStack->beginMacro("copymove");
		for (unsigned int i =0; i<items.size(); i++)
		{
			reVar val = items[i];			
			if (val.isMember("addr"))
			{
				reObject* obj = (reObject*)val["addr"].asInt();
				if (!collection->accepts(obj->className())) continue;
				reVar var; obj->toJson(var);
				reObject* clone = reRadial::shared()->assetLoader()->loadObject(var);				
				insert(clone, collection, row);
				if (action == Qt::MoveAction)
				{
					remove(obj);
				}
			}
		}
		commandStack->endMacro();
		return true;
	}
	if (collection && data->hasUrls())
	{
		commandStack->beginMacro("addref");
		foreach(const QUrl url, data->urls())
		{
			reObject* ref = reRadial::shared()->assetLoader()->loadObject(url.toLocalFile().toStdString(), true);
			if (!collection->accepts(ref->className())) continue;
			ref->path(reRadial::shared()->assetLoader()->filePath(url.toLocalFile().toStdString()));
			ref->isReference(true);
			insert(ref, collection, row < 0 ? collection->count() : row);
		}
		commandStack->endMacro();
	}
	return false;
}

Qt::DropActions reContextModel::supportedDragActions() const
{
	return Qt::CopyAction | Qt::MoveAction; // | Qt::LinkAction;
}

Qt::DropActions reContextModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction; // | Qt::LinkAction;
}

Qt::ItemFlags reContextModel::flags( const QModelIndex & index ) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
	/*if (dynamic_cast<reNodeCollection*>(object(index)) || dynamic_cast<reRenderableCollection*>(object(index)))
		return Qt::ItemIsDropEnabled | defaultFlags;
	else*/
	if (index.isValid())
	{
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	}		
	return defaultFlags;
}

void reContextModel::insert( reObject* object, reCollection* parent, int row )
{
	if (row < 0) 
	{
		row = parent->count();
	}
	beginInsertRows(index(parent), row, row);
	beginUpdates(parent);
	if (row != parent->count())
	{
		parent->insert(object, row);
	}
	else
	{
		parent->add(object);
	}
	endUpdates();
	endInsertRows();	
}

void reContextModel::remove( reObjectList& objects )
{		
	
	emit structureAboutToBeChanged();
	reObjectList supers;
	for (unsigned int i =0; i<objects.size(); i++)
	{
		reObject* obj = objects[i];
		reCollection* super = obj->super;
		bool pass = false;
		for (int n=0; n<supers.size(); n++)
		{
			if (((reCollection*)supers[n])->isChild(objects[i]))
			{
				pass = true;
				break;
			}
		}
		if (!pass)
		{
			supers.push_back(super);
		}
	}
	beginUpdates(supers);
	for (unsigned int i =0; i<objects.size(); i++)
	{
		reObject* obj = objects[i];
		int row = obj->super->indexOf(obj);
		beginRemoveRows(index(obj->super), row, row);
		obj->super->remove(obj);
		endRemoveRows();
		delete obj;
	}
	endUpdates();
}

void reContextModel::remove( reObject* obj )
{
	reObjectList list;
	list.push_back(obj);
	remove(list);
}

void reContextModel::beginUpdates( reObjectList& objects )
{
	assert(!stateMap);
	stateMap = new reStateMap;
	for (unsigned int i =0; i<objects.size(); i++)
	{
		(*stateMap)[objects[i]] = objects[i]->state();		
	}
	commandStack->beginMacro("Updates");
}

void reContextModel::beginUpdates( reObject* object )
{
	reObjectList list;
	list.push_back(object);
	beginUpdates(list);	
}

void reContextModel::endUpdates()
{
	reStateMap *currentMap = new reStateMap;
	for (reStateMap::iterator it = (*stateMap).begin(); it != (*stateMap).end(); it++)
	{
		(*currentMap)[it->first] = it->first->state();
	}
	emit stateChanged();
	commandStack->push(new reContextUpdateCommand(this, stateMap, currentMap));
	commandStack->endMacro();	
	stateMap = 0;
}

void reContextModel::cancelUpdates()
{
	for (reStateMap::iterator it = (*stateMap).begin(); it != (*stateMap).end(); it++)
	{
		it->first->state(it->second);
	}
	delete stateMap;
	stateMap = 0;
}

int indexOfName(reVar& list, string& name)
{
	for (unsigned int i =0; i<list.size(); i++)
	{
		if (list[i]["name"].asString() == name)
			return i;
	}
	return -1;
}

void reContextModel::sync( reCollection* collection, reVar prev, reVar next )
{
	reVar pc = prev["children"];
	reVar nc = next["children"];
	reVar current = pc;
	for (int i=pc.size()-1; i>-1; i--)
	{
		if (indexOfName(nc, pc[i]["name"].asString()) == -1)
		{
			beginRemoveRows(index(collection), i, i);
			collection->remove(collection->at(i));
			current.removeMember(i);
			endRemoveRows();			
		}
	}	
	int off = 0;
	for (unsigned int i =0; i<nc.size(); i++)
	{
		if (nc[i]["name"].asString() != current[i-off]["name"].asString())
		{			
			beginInsertRows(index(collection), i, i);
			collection->insert(reRadial::shared()->assetLoader()->loadObject(nc[i]), i);
			//next["children"].removeMember(i);
			endInsertRows();
			off++;
		}
	}
	collection->state(next);
	emit stateChanged();
}

reCollection * reContextModel::root() const
{
	return (reCollection*)base->at(0);
}

void reContextModel::root( reCollection * val )
{
	_root = val;
	delete base;
	base = new reCollection();
	base->add(val);
}

//////////////////////////////////////////////////////////////////////////

reContextUpdateCommand::reContextUpdateCommand( reContextModel* model, reStateMap* previous, reStateMap* next )
{
	this->previous = previous;
	this->next = next;
	this->model = model;
	done = true;
}

reContextUpdateCommand::~reContextUpdateCommand()
{
	qDebug() << "command destroyed";
	delete next, previous;
}

void reContextUpdateCommand::undo()
{
	for (reStateMap::iterator it = previous->begin(); it != previous->end(); it++)
	{		
		if (reCollection* coll = dynamic_cast<reCollection*>(it->first))
		{
			model->sync(coll, (*next)[it->first], it->second);
		} else
			it->first->state(it->second);
	}	
}

void reContextUpdateCommand::redo()
{
	if (done) 
	{
		done = false;
		return;
	}
	for (reStateMap::iterator it = next->begin(); it != next->end(); it++)
	{		
		if (reCollection* coll = dynamic_cast<reCollection*>(it->first))
		{
			model->sync(coll, (*previous)[it->first], it->second);
		} else
			it->first->state(it->second);
	}
}
