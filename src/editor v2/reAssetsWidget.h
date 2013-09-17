#ifndef reAssetsWidget_h__
#define reAssetsWidget_h__

#include "std.h"
#include "ui_AssetsWidget.h"

class reAssetsModel: public QFileSystemModel
{
public:
	reAssetsModel(QWidget* parent);
	virtual QStringList mimeTypes() const;
	virtual bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
};

class reAssetsWidget: public QWidget
{
	Q_OBJECT
public:
	reAssetsWidget(QWidget* parent = 0);	
private:
	Ui::AssetsWidget ui;
	reAssetsModel* model;
	QFileSystemWatcher* watcher;
	void watch(QString path, bool recursive);
public slots:
	void projectChanged();
	void doubleClicked();

	void fileChanged(const QString &path);
	void directoryChanged(const QString &path);

	void importFBX(QString path);
};

#endif // reAssetsWidget_h__