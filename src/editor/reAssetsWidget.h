#ifndef REASSETSWIDGET_H
#define REASSETSWIDGET_H

#include <gl/glew.h>
#include <QWidget>
#include <QTreeView>
#include <QTGui>

#include <QModelIndex>
#include <QVariant>

#include <QIcon>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QFileSystemWatcher>

#include "reAsset.h"
#include "reAssetsModel.h"


class reProject;

class reProjectModel: public QFileSystemModel
{
public:
	reProjectModel(QObject* parent);
};

class reAssetsWidget: public QWidget
{
	Q_OBJECT

public:
	reAssetsWidget(QWidget* parent = NULL);
	void updateItemPanel(reAsset* item);
private:
	QTreeView *tree;
	reAssetModel *model;
	QList<QFileSystemWatcher*> watchers;
	void watch(QString path);

private slots: 
	void projectChanged(reProject* project);
	void treeSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );
	void itemDoubleClicked (const QModelIndex&);
	void directoryChanged(const QString &);
signals:
	void assetSelectionChanged (reAsset* item);	
	void assetDoubleClicked (reAsset* asset);
};

#endif