#ifndef reAssetsModel_h__
#define reAssetsModel_h__

#include "editor.h"
#include "reFileAsset.h"

class reAssetModel:
	public QAbstractItemModel 
{	
public:
	reFileAsset *root;
	QFileIconProvider iconProvider;

	reAssetModel(QObject *parent = 0);
	~reAssetModel();

	Qt::DropActions supportedDropActions() const;
	int	rowCount ( const QModelIndex & parent = QModelIndex() ) const;

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index( QString path ) const;
	QModelIndex index( reAsset* item ) const;
	QModelIndex parent(const QModelIndex &index) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	// bool hasChildren(const QModelIndex &parent = QModelIndex()) const;	
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	reAsset* node(QModelIndex index) const;
	void refresh(QString path);
	void refresh(reAsset* item);
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
};
#endif // reAssetsModel_h__
