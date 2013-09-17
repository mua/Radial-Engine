#include "reContextWindow.h"

#include "rePreviewWidget.h"
#include "reSceneGraph.h"
#include "reViewset.h"
#include "reEditor.h"
#include "reRadial.h"
#include "reAssetLoader.h"

#include "reContext.h"

#include "reGizmo.h"
#include "reHelper.h"
#include "reViewport.h"
#include "reModifier.h"
#include "reSelectModifier.h"
#include "reTransformModifier.h"
#include "reTerrainModifier.h"
#include "reTerrainNode.h"

#include <QAction>
#include <QActionGroup>
#include <QBoxLayout>
#include <QSettings>
#include <QFileInfo>
#include <QHeaderView>

#include "reTerrainPanel.h"

reContextWindow::reContextWindow( QWidget* parent/*=0*/ ):
	QMainWindow(parent)
{
	context = 0;
	model = new reSceneModel();
	splitter = new QSplitter(this);
	sceneGraphWidget = new reSceneGraphWidget(this);
	splitter->addWidget(sceneGraphWidget);
	splitter->addWidget(viewset = new reViewset(this));

	setCentralWidget(splitter);

	connect(reEditor::instance(), SIGNAL(closing()), SLOT(closing()));
	connect(viewset, SIGNAL(nodeClicked(reNode*)), SLOT(nodeClicked(reNode*)));
	
	QSettings settings;

	helperNode = new reNode();
	viewset->helperNode(helperNode);
	selectionHelper = new reSelectionHelper(this);

	selectModifier = new reSelectModifier(this);
	selectModifier->attach(this);
	//connect(selectModifier, SIGNAL(nodesSelected(reNodeVector&)), SLOT(nodesSelected(reNodeVector&)));

	createActions();
	activeModifier = 0;

	//loadSettings();
	setDockNestingEnabled(true);

}

void reContextWindow::createActions()
{
	modifierGroup = new QActionGroup(this);
	selectAct = new QAction(QIcon(":/resources/select.png"), "Select", this);
	modifierGroup->addAction(selectAct);
	selectAct->setCheckable(true);
	translateAct = new QAction(QIcon(":/resources/move.png"), "Translate", this);	
	modifierGroup->addAction(translateAct);
	translateAct->setCheckable(true);
	rotateAct = new QAction(QIcon(":/resources/rotate.png"), "Rotate", this);
	modifierGroup->addAction(rotateAct);
	rotateAct->setCheckable(true);
	scaleAct = new QAction(QIcon(":/resources/scale.png"), "Scale", this);
	modifierGroup->addAction(scaleAct);
	scaleAct->setCheckable(true);
	terrainAct = new QAction(QIcon(":/resources/map.png"), "Terrain", this);
	modifierGroup->addAction(terrainAct);
	terrainAct->setCheckable(true);

	selectAct->setChecked(true);
	connect(modifierGroup, SIGNAL(triggered(QAction*)), SLOT(modifierActionTriggered(QAction*)));

	addAct = new QAction(QIcon(":/resources/add.png"), "Add node", this);
	connect(addAct, SIGNAL(triggered()), SLOT(addNode()));	

	deleteAct = new QAction(QIcon(":/resources/delete.png"), "Delete node", this);
	connect(deleteAct, SIGNAL(triggered()), SLOT(deleteSelected()));

	insertTerrainAct = new QAction("Insert terrain", this);
	connect(insertTerrainAct, SIGNAL(triggered()), SLOT(insertTerrain()));
}

void reContextWindow::modifierActionTriggered( QAction *action )
{
	updateModifier();
}

void reContextWindow::updateModifier()
{
	if (activeModifier)
	{
		activeModifier->detach();
		delete activeModifier;
		activeModifier = 0;
	}
	if (modifierGroup->checkedAction() == translateAct)
	{
		activeModifier = new reTranslateModifier(this);
	}
	if (modifierGroup->checkedAction() == scaleAct)
	{
		activeModifier = new reScaleModifier(this);
	}
	if (modifierGroup->checkedAction() == rotateAct)
	{
		activeModifier = new reRotateModifier(this);
	}
	if (modifierGroup->checkedAction() == terrainAct)
	{
		activeModifier = new reTerrainModifier(this);
	}
	if (activeModifier)
	{
		activeModifier->attach(this);
	}
}

void reContextWindow::closeEvent( QCloseEvent * e )
{
	closing();
}

void reContextWindow::closing()
{
	QSettings settings;
	settings.setValue("contexSplitterGeometry", splitter->saveGeometry());
	settings.setValue("contexSplitterState", splitter->saveState());
	settings.setValue("contexGeometry", saveGeometry());
	settings.setValue("contexState", saveState());
}

reNode* reContextWindow::node() const
{
	return _node;
}

void reContextWindow::node( reNode* val )
{
	_node = val;
	model->root(val);
	if (context)
	{
		delete context;
		context = 0;
	}
	context = new reContext(val);
	sceneGraphWidget->context(context);
	viewset->node(val);
	connect(context->selectionModel, 
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	redoAction = context->commandStack.createRedoAction(this, "Redo");
	redoAction->setIcon(QIcon(":/resources/redo.png"));
	undoAction = context->commandStack.createUndoAction(this, "Undo");
	undoAction->setIcon(QIcon(":/resources/undo.png"));
	connect(undoAction, SIGNAL(triggered()), SLOT(undoRedo()));
	connect(redoAction, SIGNAL(triggered()), SLOT(undoRedo()));
}

void reContextWindow::loadFile( string& fileName )
{
	if (!QFileInfo(fileName.c_str()).exists()) return;
	node((reNode*)reRadial::shared()->assetLoader()->loadObject(fileName));
	QSettings settings;	
	settings.setValue("sceneFile", fileName.c_str());
	loadedFile = fileName.c_str();
}

void reContextWindow::nodeClicked( reNode* node)
{
	/*
	reAsset* asset = model->assetByNode(node, model->root);
	if (asset)
	{
		sceneGraphWidget->selectAsset(asset);
	}	
	qDebug() << "click";
	*/
}

void reContextWindow::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
	updateModifier();
}

void reContextWindow::loadSettings()
{
	
	QSettings settings;	
	
	restoreGeometry(settings.value("contexGeometry").toByteArray());
	restoreState(settings.value("contexState").toByteArray());
	splitter->restoreGeometry(settings.value("contexSplitterGeometry").toByteArray());
	splitter->restoreState(settings.value("contexSplitterState").toByteArray());
}

void reContextWindow::deleteSelected()
{
	context->deleteObjects(context->selectedObjects());
}

void reContextWindow::addNode()
{
	reNode *parent = context->selectedObjects().size() ? context->selectedObjects()[0] : context->root;
	reNode *node = new reNode();
	node->name("newNode");
	context->addObject(parent, node);
}

void reContextWindow::transformStarted()
{
	context->beginUpdatingObjects(context->selectedObjects());
}

void reContextWindow::transformEnded()
{
	context->endUpdatingObjects();
}

void reContextWindow::undoRedo()
{
	updateModifier();
}

void reContextWindow::save( QString fileName /*= ""*/ )
{
	if (fileName.isEmpty())
	{
		fileName = loadedFile;
	}
	context->root->save(fileName.toStdString());
	loadedFile = fileName;
}

void reContextWindow::insertTerrain()
{
	reNode *parent = context->selectedObjects().size() ? (reNode*)context->selectedObjects()[0] : context->root;
	reNode *node = new reTerrainNode();
	node->name("Terrain");
	context->addObject(parent, node);
}