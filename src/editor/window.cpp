#include "window.h"

#include <QtGui>
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <QApplication>

#include "reAssetsWidget.h"
#include "reEditor.h"
#include "rePropertyWidget.h"
#include "rePreviewWidget.h"
#include "reMeshAsset.h"
#include "reNodeAsset.h"
#include "reSceneGraph.h"
#include "reContextWindow.h"
#include "reEditor.h"
#include "reContext.h"

#include <string>

using namespace std;

MainWindow::MainWindow()
{
	_currentWindow = 0;
	mdiArea = new QMdiArea;
	mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea->setViewMode(QMdiArea::TabbedView);
	mdiArea->setTabsMovable(true);
	mdiArea->setTabsClosable(true);
	mdiArea->setTabPosition(QTabWidget::TabPosition::South);

	setCentralWidget(mdiArea);
	connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
		this, SLOT(subWindowActivated(QMdiSubWindow*)));

	setDockNestingEnabled(true);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();

    setWindowTitle(tr("Dock Widgets"));

    setUnifiedTitleAndToolBarOnMac(true);

	QSettings settings;
	reEditor::instance()->project(new reProject(
		settings.value("project", "").toString().toStdString()));
	restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
	restoreState(settings.value("mainWindowState").toByteArray());	

	foreach(QString path, settings.value("openFiles", "").toString().split(","))
	{
		if (path.length())
		{
			loadContext(path);
		}		
	}
	mdiArea->setDocumentMode(true);
	
	updateCurrentWindow();

	//mdiArea->setActiveSubWindow(mdiArea->subWindowList().at(0));
	/*
	if (mdiArea->subWindowList().size())
	{
		subWindowActivated(mdiArea->subWindowList().at(0));
	}
	*/
}

void MainWindow::createActions()
{
    newProjectAct = new QAction(QIcon(":/resources/new.png"), tr("&Set Project Directory"), this);
    newProjectAct->setShortcuts(QKeySequence::New);
    newProjectAct->setStatusTip(tr("Create a new project"));
    connect(newProjectAct, SIGNAL(triggered()), this, SLOT(newProject()));

	saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save..."), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save current entity"));
	connect(saveAct, SIGNAL(triggered()), SLOT(saveContext()));

	saveAsAct = new QAction(QIcon(":/resources/save.png"), tr("&Save As..."), this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save current entity as"));
	connect(saveAsAct, SIGNAL(triggered()), SLOT(saveAsContext()));

	openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open entity"));
	connect(openAct, SIGNAL(triggered()), SLOT(openContext()));

	newAct = new QAction(QIcon(":/resources/new.png"), tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create new entity"));
	connect(newAct, SIGNAL(triggered()), SLOT(newContext()));

	quitAct = new QAction(tr("&Quit"), this);
	quitAct->setShortcuts(QKeySequence::Quit);
	quitAct->setStatusTip(tr("Quit the application"));
	//connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newProjectAct);
	fileMenu->addSeparator();
	fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);


    editMenu = menuBar()->addMenu(tr("&Edit"));    

    viewMenu = menuBar()->addMenu(tr("&View"));

	insertMenu = menuBar()->addMenu(tr("&Insert"));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    //helpMenu->addAction(aboutAct);
    //helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolbar = addToolBar(tr("File"));
	fileToolbar->setObjectName("mainToolbar");
	fileToolbar->setIconSize(QSize(16, 16));

//    fileToolBar->addAction(newProjectAct);
	//fileToolbar->addAction(aboutAct);
	fileToolbar->addAction(newAct);
	fileToolbar->addAction(openAct);
    fileToolbar->addAction(saveAct);

    editToolbar = addToolBar(tr("Edit"));
	editToolbar->setObjectName("editToolbar");
	editToolbar->setIconSize(QSize(16, 16));

	modifyToolbar = addToolBar(tr("Modify"));
	modifyToolbar->setObjectName("modifyToolbar");
	modifyToolbar->setIconSize(QSize(16, 16));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("Assets"), this);

    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    assetsWidget = new reAssetsWidget(dock);
	connect(assetsWidget, SIGNAL(assetSelectionChanged(reAsset*)), SLOT(assetSelectionChanged(reAsset*)));
	connect(assetsWidget, SIGNAL(assetDoubleClicked(reAsset*)), SLOT(assetDoubleClicked(reAsset*)));

	dock->setObjectName("Assets");
    dock->setWidget(assetsWidget);
    addDockWidget(Qt::RightDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());

	dock = new QDockWidget(tr("Properties"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	propertyWidget = new rePropertyWidget(dock);
	dock->setObjectName("Properties");
	dock->setWidget(propertyWidget);
	addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
	/*
    dock = new QDockWidget(tr("Preview"), this);
	dock->setObjectName("Preview");
	preview = new rePreviewWidget(this);
    dock->setWidget(preview);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
	*/
}

void MainWindow::newProject()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty())
	{
		reEditor::instance()->project(new reProject(dir.toStdString()));
		qDebug() << reEditor::instance()->project()->root().c_str();
	}
}

void MainWindow::closeEvent( QCloseEvent * event)
{
	reEditor::instance()->shutdown();
	QSettings settings;
	settings.setValue("project", reEditor::instance()->project()->root().c_str());
	settings.setValue("mainWindowGeometry", saveGeometry());
	settings.setValue("mainWindowState", saveState());

	QStringList list;
	foreach (QMdiSubWindow* sub, mdiArea->subWindowList())
	{
		list.append(((reContextWindow*)sub->widget())->loadedFile);
	}
	settings.setValue("openFiles", list.join(","));	
}

void MainWindow::assetSelectionChanged( reAsset* item )
{
	if (item)
	{
		//propertyWidget->loadAssetProperties(item);
		//propertyWidget->show();
	}
	else
	{
		//propertyWidget->hide();
	}
	reMeshAsset* meshAsset;
	if (meshAsset = dynamic_cast<reMeshAsset*>(item))
	{
		qDebug() << "mesh selected";
		meshAsset->mesh = new reMesh();
		meshAsset->mesh->load(meshAsset->path().toStdString());
		preview->view(meshAsset->mesh);
	}
	reNodeAsset* nodeAsset;
	if (nodeAsset = dynamic_cast<reNodeAsset*>(item))
	{
		qDebug() << "mesh selected";
		preview->view(nodeAsset->node());
		//sceneGraph->node(nodeAsset->node());
	}
}

void MainWindow::assetDoubleClicked( reAsset* asset )
{
	qDebug() << "asset selected";
	reFileAsset* fa = dynamic_cast<reFileAsset*>(asset);
	if (fa)
	{
		QFileInfo info(fa->path());
		if (info.completeSuffix().toLower() == "json") 
		{
			loadContext(fa->path());
		}
	}
}

void MainWindow::addContextWindow( reContextWindow* context )
{	
	connect(context->context->selectionModel, 
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	mdiArea->addSubWindow(context);
	context->show();
	context->loadSettings();
}

void MainWindow::loadContext(QString fileName)
{
	reContextWindow* context;
	context = new reContextWindow(this);
	context->loadFile(fileName.toStdString());
	context->setWindowTitle(fileName);
	addContextWindow(context);	
}

void MainWindow::subWindowActivated( QMdiSubWindow * window )
{
	updateCurrentWindow();
}

void MainWindow::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{	
	QMdiSubWindow* sub = mdiArea->activeSubWindow() ? mdiArea->activeSubWindow() : mdiArea->subWindowList().at(0);
	reContextWindow *contextWindow = dynamic_cast<reContextWindow*>(sub->widget());
	propertyWidget->show();
	propertyWidget->loadProperties(contextWindow->context, contextWindow->context->selectedObjects());
}

void MainWindow::openContext()
{

	QString file = QFileDialog::getOpenFileName(this, tr("Open Entity File"), reEditor::instance()->project()->root().c_str(), "*.json");
	if (!file.isEmpty())
	{
		loadContext(file);
	}
}

void MainWindow::newContext()
{
	reContextWindow* context;
	context = new reContextWindow(this);
	reNode* node = new reNode;
	node->name("NewNode");
	context->node(node);
	addContextWindow(context);
}

void MainWindow::saveContext()
{
	if (currentWindow())
	{
		if (currentWindow()->loadedFile.isEmpty())
		{
			saveAsContext();
			return;
		}
		statusBar()->showMessage("saving...");
		currentWindow()->save();
		statusBar()->showMessage("saved !");
	}
}

void MainWindow::saveAsContext()
{
	if (currentWindow())
	{
		QString file = QFileDialog::getSaveFileName(this, tr("Save Entity File"), reEditor::instance()->project()->root().c_str(), "*.json");
		if (!file.isEmpty())
		{
			statusBar()->showMessage("saving...");
			currentWindow()->save(file);
			statusBar()->showMessage("saved !");
		}
	}
}

void MainWindow::about()
{
	mdiArea->setActiveSubWindow(mdiArea->subWindowList().at(0));
	/*
	QFile f("C:\\Users\\utku\\Desktop\\cmake\\src\\editor\\resources\\style.css");
	if (f.open(QIODevice::ReadOnly)) {
		((QApplication*)QApplication::instance())->setStyleSheet(QString(f.readAll()));
		f.close();
	}
	*/
}

void MainWindow::updateCurrentWindow()
{
	QMdiSubWindow* sub = mdiArea->activeSubWindow() ? mdiArea->activeSubWindow() : 0;
	if (!sub && mdiArea->subWindowList().size())
	{
		sub = mdiArea->subWindowList().at(0);
	}
	currentWindow(sub ?  dynamic_cast<reContextWindow*>(sub->widget()) : 0);
}

reContextWindow* MainWindow::currentWindow() const
{
	return _currentWindow;
}

void MainWindow::currentWindow( reContextWindow* val )
{
	if (_currentWindow != val)
	{
		_currentWindow = val;
		currentWindowChanged();
	}	
}

void MainWindow::currentWindowChanged()
{
	editToolbar->clear();
	modifyToolbar->clear();
	insertMenu->clear();
	reContextWindow *context = currentWindow();
	if (context)
	{
		context->show();
		editToolbar->addAction(context->addAct);
		editToolbar->addAction(context->deleteAct);
		editToolbar->addSeparator();
		editToolbar->addAction(context->undoAction);
		editToolbar->addAction(context->redoAction);

		modifyToolbar->addAction(context->selectAct);
		modifyToolbar->addAction(context->translateAct);
		modifyToolbar->addAction(context->rotateAct);
		modifyToolbar->addAction(context->scaleAct);
		modifyToolbar->addSeparator();
		modifyToolbar->addAction(context->terrainAct);

		insertMenu->addAction(context->addAct);
		insertMenu->addAction(context->insertTerrainAct);
	}
	if (context)
	{
		foreach(QMdiSubWindow* sub, mdiArea->subWindowList())
		{
			if (sub->widget() != context)
			{
				sub->widget()->hide();
			}
		}
	}
}

void MainWindow::showEvent( QShowEvent * e )
{

//	reNodeVector v; v.push_back(currentWindow()->context->root->children[0]);
//	currentWindow()->context->selectNodes(v);

//	currentWindow()->terrainAct->trigger();
}