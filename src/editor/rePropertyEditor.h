#ifndef rePropertyEditor_h__
#define rePropertyEditor_h__

#include "reNode.h"
#include "rePropertySet.h"

#include <QWidget>
#include <QString>
#include <QAbstractItemModel>
#include <QCombobox>
#include <QTreeView>

class reContext;

class reNodeProperties
{
public:
	reNodeVector nodes;
	rePropertySet all;
	rePropertySet transform;
	rePropertySet worldTransform;

	reNodeProperties ();
	void createProperties(reNode* node);
	void setNodes(reNodeVector& nodes);
};


class rePropertiesModel: public QAbstractItemModel
{
public:
	rePropertySet* root;
	reContext* ctx;

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(rePropertySet* set) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	rePropertySet *set( const QModelIndex& index) const;
	Qt::ItemFlags flags( const QModelIndex &index ) const;

	void reload();
};

class reNodeEditor:
	public QWidget
{
	Q_OBJECT	
public:	
	reNodeProperties* properties;
	reNodeVector nodes;
	rePropertiesModel* model;
	reNodeEditor(QWidget* parent, reContext* ctx, reNodeVector& nodes);
public slots:
	void nodesUpdated(reNodeVector& nodes);
};

class rePropertyTree: public QTreeView
{
	Q_OBJECT	
public:
	void mousePressEvent(QMouseEvent *event);
	rePropertyTree(QWidget* parent);
};

class QBooleanComboBox : public QComboBox
{
	Q_OBJECT
		Q_PROPERTY(bool value READ value WRITE setValue USER true)

public:
	QBooleanComboBox(QWidget *parent);
	void setValue(bool);
	bool value() const;
};

#endif // rePropertyEditor_h__
