#include "std.h"
#include "reMainWindow.h"
#include "reContextWidget.h"
#include "reMaker.h"

reMainWindow::reMainWindow()
{
	loaded = 0;
	ui.setupUi(this);
	QSettings settings;
	restoreGeometry(settings.value(QString("%1Geometry").arg(objectName())).toByteArray());
	restoreState(settings.value(QString("%1State").arg(objectName())).toByteArray());
	reMaker::shared()->root(settings.value("projectFolder").toString());
	connect(ui.actionOpen, SIGNAL(triggered()), SLOT(openObject()));
	reMaker::shared()->initGL(this);
}

void reMainWindow::closeEvent( QCloseEvent * e )
{
	reMaker::shared()->shutdown();
	QSettings settings;
	settings.setValue("projectFolder", reMaker::shared()->root());
	settings.setValue(QString("%1Geometry").arg(objectName()), saveGeometry());
	settings.setValue(QString("%1State").arg(objectName()), saveState());
	QStringList openFiles;
	foreach(QMdiSubWindow* sub, ui.mdiArea->subWindowList())
	{
		if (reContextWidget *ctx = dynamic_cast<reContextWidget*>(sub->widget()))
		{
			ctx->close();
			if (!ctx->filePath.isEmpty())
				openFiles.append(ctx->filePath);
		}
	}
	settings.setValue(QString("%1OpenFiles").arg(objectName()), openFiles);
	QMainWindow::closeEvent(e);
}

void reMainWindow::newObject()
{	
	reContextWidget* ctx = new reContextWidget(this);
	ui.mdiArea->addSubWindow(ctx);
	ctx->loadSettings();
	ctx->show();
	ctx->newContext();
	connect(ctx->selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(updateProperties()));	
}

void reMainWindow::openObject(QString path)
{
	if (path.isEmpty())
	{
		path = QFileDialog::getOpenFileName(this, "Set project", reMaker::shared()->root(), "*.json");
	}	
	if (!path.isEmpty())
	{
		reContextWidget* ctx = new reContextWidget(this);
		ui.mdiArea->addSubWindow(ctx);
		ctx->loadSettings();
		ctx->show();
		ctx->open(path);
		connect(ctx->selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(updateProperties()));	
	}
}

void reMainWindow::setProject()
{
	QString dir = QFileDialog::getExistingDirectory(this, "Set project", "", QFileDialog::ShowDirsOnly);
	if (!dir.isEmpty())
	{
		reMaker::shared()->root(dir);
	}
}

void reMainWindow::showEvent( QShowEvent * e )
{
	if (loaded)
	{
		return;
	}
	
	QSettings settings;
	QStringList openFiles = settings.value(QString("%1OpenFiles").arg(objectName())).toStringList();
	foreach (QString path, openFiles)
	{
		openObject(path);
	}
	loaded = true;
}

void reMainWindow::subWindowActivated( QMdiSubWindow* sub )
{	
	QWidget* widget = sub ? sub->widget() : 0;
	if (!widget && !isActiveWindow()) return;
	
	ui.childToolbar->clear();
	updateProperties();

	if (reContextWidget* ctx =  dynamic_cast<reContextWidget*>(widget))
	{
		//disconnect(this, SLOT(updateProperties()));		
		foreach (QAction* act, ctx->actions())
		{
			ui.childToolbar->addAction(act);
		}
		ctx->updateToolbar(ui.childToolbar);
	}
	//qDebug() << sub->widget()->windowTitle();
}

void reMainWindow::updateProperties()
{
	qDebug() << ui.mdiArea->activeSubWindow();	
	reObjectList selected;
	QMdiSubWindow* sub = ui.mdiArea->activeSubWindow();
	QWidget* widget = sub ? sub->widget() : 0;
	reContextWidget* ctx;
	if (ctx =  dynamic_cast<reContextWidget*>(widget))
	{
		selected = ctx->selectionModel->selectedObjects();
	}
	ui.propertyWidget->setObjects(selected);
	ui.propertyWidget->contextModel(ctx ? ctx->model : 0);
}
