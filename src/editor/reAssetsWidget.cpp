#include "reAssetsWidget.h"
#include "reEditor.h"
#include "reProject.h"
#include "reMeshAsset.h"

reProjectModel::reProjectModel( QObject* parent ):
	QFileSystemModel(parent)
{
	
}

//////////////////////////////////////////////////////////////////////////

reAssetsWidget::reAssetsWidget(QWidget *parent):
QWidget(parent)
{	
	QBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(0,0,0,0);
	tree = new QTreeView(this);
	layout->addWidget(tree);
	setLayout(layout);
	connect(reEditor::instance(), SIGNAL(projectChanged(reProject*)), SLOT(projectChanged(reProject*)));
	connect(tree, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(itemDoubleClicked(const QModelIndex&)));

	tree->setEditTriggers(QAbstractItemView::SelectedClicked);
	tree->setExpandsOnDoubleClick(false);
}

void reAssetsWidget::watch(QString path)
{
	QFileSystemWatcher* watcher =  new QFileSystemWatcher(this);
	qDebug() << "watching:" << path;
	watcher->addPath(path);
	connect(watcher, SIGNAL(directoryChanged( const QString & )), SLOT(directoryChanged( const QString & )));
	watchers.append(watcher);
}

void reAssetsWidget::projectChanged( reProject* project )
{
	model = new reAssetModel(this);
	model->root->setPath(project->root().c_str());
	//model->setRootPath(project->root().c_str());
	//tree->setRootIndex(model->index(project->root().c_str()));
	foreach (QFileSystemWatcher* watcher, watchers)
	{
		delete watcher;
	}
	watchers.clear();

	watch(QFileInfo(project->root().c_str()).absoluteFilePath());
	QDirIterator it(project->root().c_str(), QDir::Dirs | QDir::NoDotAndDotDot);
	while (it.hasNext())
	{			 		
		QFileInfo info(it.next());
		watch(info.absoluteFilePath());
	}

	tree->setModel(model);

	tree->setHeaderHidden(false);

	tree->setDragEnabled(true);	
	tree->setAcceptDrops(true);
	tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	tree->setDropIndicatorShown(true);

	//tree->setRootIndex(model->index(project->root().c_str()));
	//tree->setDragDropMode(QAbstractItemView::InternalMove);

	tree->header()->hideSection(1);
	tree->header()->hideSection(2);
	tree->header()->hideSection(3);
	tree->setHeaderHidden(true);
	connect(tree->selectionModel(), 
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		SLOT(treeSelectionChanged(const QItemSelection&, const QItemSelection&)));
	tree->expandAll();
}

void reAssetsWidget::treeSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
	/*
	qDebug() << "selection changed";	
	reAsset* item = NULL;
	if (selected.indexes().count())
	{
		
		item = (reAsset*)(selected.indexes().at(0).internalPointer());
		if (item)
		{
			//qDebug() << item->path();	
		}
		
	} else {
		item = NULL;
	}	
	emit assetSelectionChanged(item);
	updateItemPanel(item);
	*/
}

void reAssetsWidget::updateItemPanel(reAsset* item)
{
	
}

void reAssetsWidget::itemDoubleClicked( const QModelIndex& idx)
{
	qDebug() << "double click";
	emit assetDoubleClicked(model->node(idx));
}

void reAssetsWidget::directoryChanged( const QString & path)
{
	model->refresh(path);
}