#ifndef reContextModel_h__
#define reContextModel_h__

#include <gl/glew.h>
#include "reAssetsModel.h"
#include "reNodeAsset.h"

#include <QUndoStack>

class reNodeModel:
	public reAssetModel
{
public:
	QUndoStack commandStack;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		int row, int column, const QModelIndex &_parent);
	reNodeModel(QObject *parent = 0);
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	reNodeAsset* assetByNode(reNode* enode, reAsset* parent);
	void removeAsset( reAsset* asset );
	//	Qt::DropActions supportedDropActions() const;
	//	Qt::DropActions supportedDragActions() const;
};

//////////////////////////////////////////////////////////////////////////

class reSceneAssetCommand: 
	public QUndoCommand
{
public:
	reAsset *asset;
	reNodeModel* model;
	int row;
	reSceneAssetCommand(reAsset* asset, reNodeModel* model, QUndoCommand *parent = 0);
	virtual void undo()=0;
	virtual void redo()=0;
};

class reSceneAssetDeleteCommand: public reSceneAssetCommand
{
public:
	reSceneAssetDeleteCommand(reAsset* asset, reNodeModel* model, QUndoCommand *parent = 0);
	virtual void undo();
	virtual void redo();
};

class reSceneAssetAddCommand: public reSceneAssetCommand
{
public:
	reSceneAssetAddCommand(reAsset* asset, reNodeModel* model, QUndoCommand *parent = 0);
	virtual void undo();
	virtual void redo();
};

class reSceneAssetMoveCommand: public reSceneAssetCommand
{
public:
	reAsset *target, *from;
	reSceneAssetMoveCommand(reAsset* asset, reAsset* target, reNodeModel* model, QUndoCommand *parent=0);
	virtual void undo();
	virtual void redo();
};

#endif // reContextModel_h__
