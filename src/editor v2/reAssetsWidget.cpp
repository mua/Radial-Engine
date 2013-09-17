#include "std.h"
#include "reAssetsWidget.h"
#include "reMaker.h"
#include "reObject.h"
#include "reMainWindow.h"

#include "reImporter.h"

reAssetsModel::reAssetsModel( QWidget* parent ):
	QFileSystemModel(parent)
{

}

QStringList reAssetsModel::mimeTypes() const
{
	QStringList list = QFileSystemModel::mimeTypes();
	list.append("text/json");
	return 	list;
}

bool reAssetsModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
	QFileInfo info(QFileSystemModel::filePath(parent));
	if (data->hasFormat("text/json"))
	{
		QDir dir = info.isDir() ? QDir(info.filePath()) : info.dir();
		QString json = QString(data->data("text/json"));
		Json::Reader reader;
		Json::Value items;
		reader.parse(json.toStdString(), items);
		for (unsigned int i =0; i<items.size(); i++)
		{
			reVar val = items[i];
			if (val.isMember("addr"))
			{
				reObject* obj = (reObject*)val["addr"].asInt();		
				QString fileName = QString("%1.%2.json").arg(obj->name().c_str(), obj->className().c_str());
				obj->saveToFile(dir.absoluteFilePath(fileName).toStdString());
				obj->path(dir.relativeFilePath(fileName).toStdString());
			}
		}
		return true;
	}
	return QFileSystemModel::dropMimeData(data, action, row, column, parent);
}

//////////////////////////////////////////////////////////////////////////

reAssetsWidget::reAssetsWidget( QWidget* parent /*= 0*/ )
{
	ui.setupUi(this);
	model = new reAssetsModel(this);	
	model->setReadOnly(false);
	connect(reMaker::shared(), SIGNAL(projectSet()), SLOT(projectChanged()));
	projectChanged();
	watcher = new QFileSystemWatcher();
	connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(directoryChanged(const QString&)));
	connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(const QString&)));
}

void reAssetsWidget::projectChanged()
{
	if (!reMaker::shared()->root().isEmpty())
	{
		ui.tree->setModel(model);
		model->sort(0);
		ui.tree->hideColumn(1);
		ui.tree->hideColumn(2);
		ui.tree->hideColumn(3);
		
		ui.tree->setDragEnabled(true);
		ui.tree->setAcceptDrops(true);
		ui.tree->setDropIndicatorShown(true);

		model->setRootPath(reMaker::shared()->root());
		QModelIndex idx = model->index(reMaker::shared()->root());
		ui.tree->setRootIndex(idx);
		watch(reMaker::shared()->root(), true);
	}
}

void reAssetsWidget::doubleClicked()
{
	reMaker::shared()->mainWindow->openObject(model->filePath(ui.tree->currentIndex()));
}

void reAssetsWidget::watch( QString path, bool recursive )
{
	qDebug() << "watching dir:" << path;
	if (!watcher->directories().contains(path))
	{
		watcher->addPath(path);
	}	
	QDir dir(path);
	foreach(QFileInfo info, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
	{
		QString path = info.absoluteFilePath();
		if (info.isDir())
		{
			watch(path, true);
		}
		if (info.suffix().toLower() == "fbx" && !watcher->files().contains(path))
		{
			importFBX(path);
			qDebug() << "watching file:" << path;
			watcher->addPath(path);
		}
	}
}

void reAssetsWidget::directoryChanged( const QString & path )
{
	watch(path, false);
}

void reAssetsWidget::fileChanged( const QString & path )
{
	qDebug() << path;
	importFBX(path);
}

void reAssetsWidget::importFBX( QString path )
{
	QFileInfo info(path);
	QDir dir = info.dir();
	QDir dataDir(dir.absoluteFilePath(QString(".meta/%1").arg(info.baseName())));
	dir.mkpath(dataDir.absolutePath());

	reFBXImpoter* importer = new reFBXImpoter;
	importer->path(path);
	importer->dataDir(dataDir.absolutePath());
	importer->import();
}