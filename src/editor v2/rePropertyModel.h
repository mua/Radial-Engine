#ifndef rePropertyModel_h__
#define rePropertyModel_h__

#include "std.h"

#include "reObject.h"
#include "reCollection.h"

class rePropertyEditor;

class rePropertyModel: public QAbstractItemModel
{
	Q_OBJECT
private:
	reObjectList _objects;

public:
	rePropertyEditor* root;
	rePropertyModel();

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(rePropertyEditor* set) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	rePropertyEditor *editor( const QModelIndex& index) const;
	Qt::ItemFlags flags( const QModelIndex &index ) const;

	void reload();
	reObjectList objects() const;
	void objects(reObjectList val);
	virtual void reset();
signals:
	void objectsAboutToBeChanged(reObjectList& objects);
	void objectsChanged();
};

#endif // rePropertyModel_h__