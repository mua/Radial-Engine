#include "std.h"
#include "reContextWidget.h"
#include "reMaker.h"
#include "reNode.h"
#include "reAssetLoader.h"
#include "reRadial.h"
#include "reSelectModifier.h"
#include "reTransformModifier.h"
#include "reViewport.h"
#include "reAnimationSet.h"
#include "rePhysics.h"
#include "reController.h"
#include "reLight.h"

//#include "modeltest.h"

reContextWidget::reContextWidget( QWidget* parent /*= 0*/ ):
	QMainWindow(parent)
{
	filePath = "";
	playState = stopped;

	ui.setupUi(this); 
	commandStack = new QUndoStack();

	model = new reContextModel();
	model->commandStack = commandStack;
	connect(model, SIGNAL(stateChanged()), SLOT(stateChanged()));

	selectionModel = new reContextSelectionModel(model);
	selectionModel->commandStack = commandStack;
	connect(selectionModel,
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
		SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	
	insertMenu = new QMenu("Add", this);

	ui.tree->setModel(model);
	ui.tree->setDragEnabled(true);
	ui.tree->setAcceptDrops(true);
	ui.tree->setSelectionModel(selectionModel);

	undoAction = commandStack->createUndoAction(this);
	undoAction->setIcon(QIcon(":/undo.png"));
	undoAction->setShortcut(QKeySequence::Undo);
	redoAction = commandStack->createRedoAction(this);
	redoAction->setIcon(QIcon(":/redo.png"));
	redoAction->setShortcut(QKeySequence::Redo);

	connect(ui.actionRemove, SIGNAL(triggered()), SLOT(removeObject()));
	connect(ui.actionInsert, SIGNAL(triggered()), SLOT(insertActionTriggered()));

	connect(ui.actionCopy, SIGNAL(triggered()), SLOT(copy()));
	connect(ui.actionCut, SIGNAL(triggered()), SLOT(cut()));
	connect(ui.actionPaste, SIGNAL(triggered()), SLOT(paste()));

	connect(ui.actionMove, SIGNAL(triggered()), SLOT(modifierTriggered()));
	connect(ui.actionRotate, SIGNAL(triggered()), SLOT(modifierTriggered()));
	connect(ui.actionScale, SIGNAL(triggered()), SLOT(modifierTriggered()));
	connect(ui.actionSelect, SIGNAL(triggered()), SLOT(modifierTriggered()));

	modifierGroup = new QActionGroup(this);
	modifierGroup->addAction(ui.actionMove);
	modifierGroup->addAction(ui.actionRotate);
	modifierGroup->addAction(ui.actionScale);
	modifierGroup->addAction(ui.actionSelect);
	activeModifier = 0;
	
	viewset = ui.viewset;
	selectModifier = new reSelectModifier(this);
	helperNode = new reNode();
	viewset->helperNode = helperNode; 
	selectModifier->attach(this);

	controlGroup = new QActionGroup(this);
	controlGroup->setExclusive(true);
	controlGroup->addAction(ui.actionPlay);
	controlGroup->addAction(ui.actionPause);
	controlGroup->addAction(ui.actionStop);

	connect(this, SIGNAL(updateModifier()), SLOT(modifierTriggered()), Qt::QueuedConnection);
}

void reContextWidget::closeEvent( QCloseEvent * e )
{
	saveSettings();	
	QMainWindow::closeEvent(e);
	ui.viewset->close();
}

void reContextWidget::loadSettings()
{
	QSettings settings;
	qDebug() << objectName();
	restoreGeometry(settings.value(QString("%1Geometry").arg(objectName())).toByteArray());
	restoreState(settings.value(QString("%1State").arg(objectName())).toByteArray());
}

void reContextWidget::saveSettings()
{
	qDebug() << objectName();
	QSettings settings;
	settings.setValue(QString("%1Geometry").arg(objectName()), saveGeometry());
	settings.setValue(QString("%1State").arg(objectName()), saveState());	
}

void reContextWidget::newContext()
{
	reCollection* base = new reCollection();
	reNode* root = new reNode();
	//root->add(new rePhysics());
	base->add(root);
	setWindowTitle("New Object");
	model->collection(base);
	ui.viewset->model(model);

	reLight* light = new reLight;
	light->angles(reVec3(-49, -32, 0));
	light->distance(8.2);
	((reNode*)root)->add(light);
}

void reContextWidget::open( QString path )
{	
	reCollection* base = new reCollection();
	reObject* root = reRadial::shared()->assetLoader()->loadObject(path.toStdString());
	rePhysics* phy = 0;
	reNode *node = dynamic_cast<reNode*>(root);
	if (node)
		phy = node->findObject<rePhysics>();
	if (!phy)
	{
		//phy = new rePhysics();
		//node->add(phy);		
	}
	if (!root)
		root = new reNode;
	base->add(root);
	setWindowTitle(QDir(reMaker::shared()->root()).relativeFilePath(filePath));
	filePath = path;
	model->collection(base);

	if (!((reNode*)root)->findObject<reLight>())
	{
		reLight* light = new reLight;
		light->angles(reVec3(-49, -32, 0));
		light->distance(8.2);
		((reNode*)root)->add(light);
	}

//	ui.tree->expand(model->index(phy));
	if (phy)
	{
		phy->initPhysics();
	}
	//selectModifier->attach(this);
	//new ModelTest(model, this);
	ui.viewset->model(model);

}

void reContextWidget::save()
{
	if (filePath.isEmpty())
	{
		filePath = QFileDialog::getSaveFileName(this, "Set project", reMaker::shared()->root(), "*.json");
	}	
	if (!filePath.isEmpty()) 
	{
		reObject* object = model->root();
		object->saveToFile(filePath.toStdString());
		setWindowTitle(filePath);
	}
}

void reContextWidget::treeContextMenu( QPoint point )
{
	QMenu *menu = new QMenu();
	if (insertMenu->actions().size())
	{
		menu->addMenu(insertMenu);
	}
	menu->exec(ui.tree->mapToGlobal(point));
}

void reContextWidget::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
	updateMenus();
	qDebug() << selectionModel->selectedObjects().size();
	modifierTriggered();
}

void reContextWidget::updateToolbar( QToolBar* toolbar )
{		
	insertButton = new QToolButton(this);
	insertButton->setPopupMode(QToolButton::MenuButtonPopup);
	insertButton->setDefaultAction(ui.actionInsert);
	insertButton->setMenu(insertMenu);	
	updateMenus();

	toolbar->addAction(ui.actionSave);
	toolbar->addSeparator();
	toolbar->addWidget(insertButton);
	toolbar->addAction(ui.actionRemove);
	toolbar->addAction(undoAction);
	toolbar->addAction(redoAction);	
	toolbar->addSeparator();
//	toolbar->addAction(ui.actionCut);
	toolbar->addAction(ui.actionCopy);
	toolbar->addAction(ui.actionPaste);	
	toolbar->addSeparator();
	toolbar->addAction(ui.actionSelect);
	toolbar->addAction(ui.actionMove);
	toolbar->addAction(ui.actionRotate);
	toolbar->addAction(ui.actionScale);
	toolbar->addSeparator();
	toolbar->addAction(ui.actionAnimate);
	toolbar->addAction(ui.actionStopAnimation);
	toolbar->addSeparator();
	toolbar->addAction(ui.actionPlay);
	toolbar->addAction(ui.actionPause);
	toolbar->addAction(ui.actionStop);
}

void reContextWidget::updateMenus()
{
	insertMenu->clear();
	ui.actionCopy->setDisabled(!selectionModel->selectedRows().size());
	ui.actionCut->setDisabled(!selectionModel->selectedRows().size());
	reObject* obj = model->object(selectionModel->currentIndex());
	if (!obj || !selectionModel->currentIndex().isValid()) 
	{
		insertButton->setDisabled(true);
		return;
	}
	ui.actionRemove->setEnabled(!obj->fixed());
	QAction* action;
	foreach (reTypeEntry* type, reMaker::shared()->types)
	{
		if (reMaker::shared()->getTypeInfo(obj->className().c_str())->accepts.contains(type->className))
		{
			action = new QAction(this);
			action->setText(QString("Add %1").arg(type->className));
			action->setData(type->className);
			connect(action, SIGNAL(triggered()), SLOT(insertActionTriggered()));
			insertMenu->addAction(action);
		}
	}
	insertButton->setDisabled(!insertMenu->actions().size());

	reSequence* seq = dynamic_cast<reSequence*>(obj);
	ui.actionAnimate->setEnabled(seq);
	ui.actionStopAnimation->setEnabled(seq);
}

void reContextWidget::insertActionTriggered()
{
	QAction* action = dynamic_cast<QAction*>(sender());
	if (action == ui.actionInsert)
	{
		if (insertMenu->actions().size())
		{
			insertMenu->actions().at(0)->trigger();
		}
	}
	else
	{
		reCollection* parent = (reCollection*)model->object(selectionModel->currentIndex());
		model->insert(reMaker::shared()->getTypeInfo(action->data().toString())->create(), parent);
	}
}

void reContextWidget::removeObject()
{
	reObjectList list;
	foreach (QModelIndex idx, selectionModel->selectedRows())
	{
		list.push_back(model->object(idx));
	}
	model->remove(list);
}

bool reContextWidget::eventFilter( QObject *object, QEvent *event )
{
	return false;
}

void reContextWidget::copy()
{
	QMimeData* data = model->mimeData(selectionModel->selectedRows());
	QApplication::clipboard()->setMimeData(data);
}

void reContextWidget::paste()
{
	const QMimeData* data = QApplication::clipboard()->mimeData();
	model->dropMimeData(data, Qt::CopyAction, 0, 0, selectionModel->selectedIndexes().at(0));
}

void reContextWidget::cut()
{

}

reNode* reContextWidget::node()
{
	return dynamic_cast<reNode*>(model->base->at(0));
}

void reContextWidget::modifierTriggered()
{
	/*
	for (unsigned int i =0; i<ui.viewset->viewports.size(); i++)
	{
		ui.viewset->viewports[i]->overNode(0);
	}
	*/
	if (activeModifier)
	{
		activeModifier->detach();
		delete activeModifier;
		activeModifier = 0;
	}
	QAction* action = modifierGroup->checkedAction();
	if (action == ui.actionMove)
	{
		activeModifier = new reTranslateModifier(this);
	}
	if (action == ui.actionRotate)
	{
		activeModifier = new reRotateModifier(this);
	}
	if (action == ui.actionScale)
	{
		activeModifier = new reScaleModifier(this);
	}
	if (activeModifier)
	{
		activeModifier->attach(this);
	}
}

void reContextWidget::stateChanged()
{
	//QApplication::postEvent() // dikkat buraya
	emit updateModifier();
}

void reContextWidget::playAnimation()
{
	reSequence* seq = dynamic_cast<reSequence*>(model->object(selectionModel->currentIndex()));
	if (seq)
	{
		reAnimator* aset = (reAnimator*)seq->super;
		aset->play(seq->name());
		start(aset);
	}	
}

void reContextWidget::stopAnimation()
{
	stop();
}

void reContextWidget::start(reController* controller)
{
	if (playState == started)
	{
		ui.actionPause->trigger();
		return;
	}
	if (playState == stopped)
	{
		model->beginUpdates(model->root());
	}
	playState = started;
	if (controller)
	{
		controller->start();
	}
	else
	{
		std::vector<reController*> controllers;
		model->root()->findObjects(controllers);
		for (size_t i=0; i<controllers.size(); i++)
		{
			controllers[i]->start();
		}
	}
	for (unsigned int i =0; i<ui.viewset->viewports.size(); i++)
	{
		//ui.viewset->viewports[i]->trackMouse(false);
	}
}

void reContextWidget::pause()
{
	if (playState == started)
	{
		playState = paused;
		std::vector<reController*> controllers;
		model->root()->findObjects(controllers);
		for (size_t i=0; i<controllers.size(); i++)
		{
			controllers[i]->pause();
		}
	}
}

void reContextWidget::stop()
{
	if (playState != stopped)
	{
		model->cancelUpdates();
		playState = stopped;
		std::vector<reController*> controllers;
		model->root()->findObjects(controllers);
		for (size_t i=0; i<controllers.size(); i++)
		{
			controllers[i]->stop();
		}	
	}
	for (unsigned int i =0; i<ui.viewset->viewports.size(); i++)
	{
		ui.viewset->viewports[i]->trackMouse(true);
	}
}
