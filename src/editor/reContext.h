#ifndef reContext_h__
#define reContext_h__

#include "reNode.h"

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QUndoStack>
#include <QUndoCommand>

class reContext;

typedef vector<reVar> reNodeStateCnt;
typedef reVar reNodeState;

class reSceneModel:
	public QAbstractItemModel
{
private:
	reNode* _root;
	reNode* base;	
public:
	reContext *context;

	reSceneModel(QObject* parent=0);
	virtual ~reSceneModel();

	reNode* root() const;
	void root(reNode* val);
	reObject* object(const QModelIndex &idx) const;

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex index(reObject* object) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &_parent);
	virtual Qt::DropActions supportedDropActions() const;
	virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
	QStringList mimeTypes() const;

	void emitNodeUpdate(reNode* anode);
};

typedef QItemSelectionModel reSceneSelectionModel;

class reContext;

class reContextCommand: public QUndoCommand
{	
public:
	reContext* ctx;
	reContextCommand(reContext* ctx);
};

class reSelectCommand: public reContextCommand
{
private:
	reObjectList objects, oldObjects;
public:
	reSelectCommand(reContext* ctx, reObjectList& nodes);
	void undo();
	void redo();
};

class reDeleteCommand: public reContextCommand
{
public:
	reObject *deletedObject;
	reCollection *parentObject;
	int row;
	reDeleteCommand(reContext *ctx, reObject* object);
	~reDeleteCommand();

	void undo();
	void redo();
};

class reAddCommand: public reContextCommand
{
public:
	reObject *addedObject;
	reCollection *parentObject;
	int row;
	reAddCommand(reContext *ctx, reCollection* parent, reObject* child, int row = -1);
	~reAddCommand();

	void undo();
	void redo();
};

class reNodeUpdateCommand: public reContextCommand
{
public:
	reObject *object;
	reObjectState *begin, *end;
	reNodeUpdateCommand(reContext *ctx, reObject* node, reObjectState *begin, reObjectState *end);
	~reNodeUpdateCommand();

	void undo();
	void redo();
};

class reContext: public QObject
{
	Q_OBJECT
private:
	reObjectList updatingObjects;
	reNodeStateCnt updatingStates;

public:
	bool commandRunning;
	reNodeVector currentSelectedObjects;
	QUndoStack commandStack;

	reSceneModel *model;
	reSceneSelectionModel *selectionModel;
	reNode* root;

	reContext(reNode* node);
	virtual ~reContext();
	reObjectList selectedObjects();

	void selectObjects( reObjectList& objects );
	void deleteObjects( reObjectList& objects );
	void addObject(reCollection* parent, reObject* child, int row = -1);
	void deleteObject( reObject* object );
	void pushObjects( reObjectList& objects );
	void beginUpdatingObjects( reObjectList& objects );
	void endUpdatingObjects();
	void emitObjectsUpdated(reObjectList& objects);

public slots:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
signals:
	void objectsUpdated(reObjectList& nodes);

};

#endif // reContext_h__