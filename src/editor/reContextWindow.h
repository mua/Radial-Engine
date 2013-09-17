#ifndef reContextWindow_h__
#define reContextWindow_h__

#include "reRadial.h"

#include <QMainWindow>
#include <QSplitter>
#include <QUndoStack>
#include <string>
#include <QItemSelection>
#include "reContext.h"

#include "reNode.h"

class reSceneGraphWidget;
class reSceneModel;
class reViewset;
class reSelectionHelper;
class reModifier;
class reSelectModifier;

class QAction;
class QActionGroup;

class reContextWindow:
	public QMainWindow
{
	Q_OBJECT
private:
	QSplitter* splitter;
	reNode* _node;			
	QActionGroup *modifierGroup;
	reModifier *activeModifier;	
	QList<QAction> toolbarActions;

public:
	QAction *selectAct, *translateAct, *rotateAct, *scaleAct, *terrainAct;	
	QAction *undoAction, *redoAction;
	reContext* context;
	QAction *deleteAct, *addAct, *insertTerrainAct;
	QString loadedFile;

	reSelectionHelper* selectionHelper;
	reNode* helperNode;
	reViewset* viewset;
	reSelectModifier *selectModifier;
	reContextWindow(QWidget* parent=0);

	void loadSettings();
	virtual void closeEvent( QCloseEvent * e );
	
	reSceneGraphWidget *sceneGraphWidget;

	reSceneModel* model;
	reNode* node() const;
	void node( reNode* val );

	void loadFile( std::string& fileName );
	void createActions();
	void updateModifier();

	void transformStarted();
	void transformEnded();
	void save(QString fileName = "");

public slots:
	void closing();
	void nodeClicked(reNode*);
	void modifierActionTriggered(QAction *action);
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void deleteSelected();
	void addNode();
	void undoRedo();
	void insertTerrain();
};

#endif // reContextWindow_h__