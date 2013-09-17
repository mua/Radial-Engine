#include "reContextModel.h"
#include "reNode.h"
#include "reRefNode.h"
#include "reRadial.h"
#include "reAssetLoader.h"

bool reNodeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &_parent)
{	
	if (!_parent.isValid())
		return false;

	reNodeAsset* to = (reNodeAsset*)node(_parent);

	if (data->text().count()) 
	{
		reNode* enode = new reNode;
		Json::Reader reader;		
		Json::Value root;
		reader.parse(data->text().toStdString(), root);
				
		reNodeAsset* ne = (reNodeAsset*)root["pointer"].asInt();
		if (ne)
		{
			commandStack.push(new reSceneAssetMoveCommand(ne, node(_parent), this));
			return false;
		}

		enode = (reNode*)reRadial::shared()->assetLoader()->loadObject(root);
		to->node()->children->add(enode);
	} 
	else 
	{	
		QList<QUrl> urls = data->urls();
		QList<QUrl>::const_iterator it = urls.constBegin();
		for (; it != urls.constEnd(); ++it)
		{
			reRefNode* rn = new reRefNode;
			rn->loadNodeFile((*it).toLocalFile().toStdString());
			int i = 1;
			string base = rn->name();
			while (to->childByName(rn->name())) 
			{
				rn->name(base+"_"+QString::number(i).toStdString());
				i++;
			}
			to->node()->children->add(rn);
		}
	}
	to->node(to->node(), true);

	reAsset* item = to;
	QModelIndex id = _parent;
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
	return true;
}

reNodeAsset* reNodeModel::assetByNode(reNode* enode, reAsset* parent)
{
	reNodeAsset* na;
	if (na = dynamic_cast<reNodeAsset*>(parent)) 
	{
		if (na->node() == enode) return na;
	}
	for (int i=0; i<parent->children.size(); i++)
	{
		na = assetByNode(enode, parent->children[i]);
		if (na)
			return na;
	}
	return NULL;
}

QMimeData *reNodeModel::mimeData(const QModelIndexList &indexes) const
{
	QList<QUrl> urls;
	QList<QModelIndex>::const_iterator it = indexes.begin();

	qDebug() << indexes.count();

	QMimeData *data = new QMimeData();

	for (; it != indexes.end(); ++it)
		if ((*it).column() == 0) { 
			reNodeAsset *ne = dynamic_cast<reNodeAsset*>(node(*it));
			if (ne)
			{
				Json::Value json = ne->node()->toJson();
				json["pointer"] = int(ne);
				qDebug() << ne->node()->toJson().toStyledString().c_str();
				data->setText(json.toStyledString().c_str());
				break;
			}
		}
	data->setUrls(urls);
	return data;
}

bool reNodeModel::setData(const QModelIndex &index, const QVariant &value, int role /* = Qt::EditRole */)
{
	if (!index.isValid() || value.toString() == "")
		return false;
	reAsset* item = (reAsset*)index.internalPointer();
	reNodeAsset *ni = dynamic_cast<reNodeAsset*>(item);
	switch (role)
	{
	case Qt::EditRole:		
		if (NULL != ni)
		{
			ni->node()->name(value.toString().toStdString());
		}
		return true;
	default:
		return false;
	}
}

void reNodeModel::removeAsset( reAsset* asset )
{
	commandStack.push(new reSceneAssetDeleteCommand(asset, this, 0));
}

/*
Qt::DropActions reNodeModel::supportedDropActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions reNodeModel::supportedDragActions() const
{
	return Qt::MoveAction;
}
*/
reNodeModel::reNodeModel(QObject *parent /* = 0 */):
	reAssetModel(parent)
{

}


	reSceneAssetCommand::reSceneAssetCommand(reAsset *asset, reNodeModel* model,  QUndoCommand *parent /*= 0*/ ):
QUndoCommand(parent)
{
	this->asset = asset;
	this->model = model;
}

void reSceneAssetDeleteCommand::undo()
{
	asset->parent->children.insert(asset->parent->children.begin() + row, 1, asset);
	model->insertRow(asset->parent->children.size()-1, model->index(asset->parent));
}

void reSceneAssetDeleteCommand::redo()
{
	QModelIndex idx = model->index(asset);
	row = idx.row();
	model->removeRow(idx.row(), idx.parent());	
	asset->parent->removeChild(asset);
}

reSceneAssetDeleteCommand::reSceneAssetDeleteCommand( reAsset* asset, reNodeModel* model, QUndoCommand *parent /*= 0*/ ):
reSceneAssetCommand(asset, model, parent)
{

}

reSceneAssetAddCommand::reSceneAssetAddCommand( reAsset* asset, reNodeModel* model, QUndoCommand *parent /*= 0*/ ):
reSceneAssetCommand(asset, model, parent)
{

}

void reSceneAssetAddCommand::undo()
{
	QModelIndex idx = model->index(asset);
	row = idx.row();
	model->removeRow(idx.row(), idx.parent());	
	asset->parent->removeChild(asset);
}

void reSceneAssetAddCommand::redo()
{
	asset->parent->addChild(asset);
	model->insertRow(0, model->index(asset->parent));	
}

reSceneAssetMoveCommand::reSceneAssetMoveCommand(reAsset* asset, reAsset* target, reNodeModel* model, QUndoCommand *parent):
reSceneAssetCommand(asset, model, parent)
{
	this->target = target;
	this->from = asset->parent;
}

void reSceneAssetMoveCommand::undo()
{
	QModelIndex idx = model->index(asset);
	row = idx.row();
	model->removeRow(idx.row(), idx.parent());
	asset->parent->removeChild(asset);

	from->addChild(asset);
	model->insertRow(row, model->index(from));
}

void reSceneAssetMoveCommand::redo()
{
	QModelIndex idx = model->index(asset);
	row = idx.row();
	model->removeRow(idx.row(), idx.parent());
	asset->parent->removeChild(asset);

	target->addChild(asset);
	model->insertRow(row, model->index(target));
}