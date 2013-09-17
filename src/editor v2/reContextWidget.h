#ifndef reContextWidget_h__
#define reContextWidget_h__

#include "std.h"
#include "ui_ContextWidget.h"

#include "reContextModel.h"
#include "reContextSelectionModel.h"

class reNode;
class reSelectModifier;
class reModifier;
class reController;

class reContextWidget: public QMainWindow
{
	Q_OBJECT
private:
	QToolButton* insertButton;
	QAction* undoAction;
	QAction* redoAction;
	QActionGroup *modifierGroup, *controlGroup;
	enum 
	{
		started, paused, stopped
	} playState;
public:
	reModifier* activeModifier;
	reNode* helperNode;
	QUndoStack* commandStack;
	QMenu* insertMenu;
	QString filePath;
	reSelectModifier* selectModifier;

	reContextWidget(QWidget* parent = 0);

	reContextModel* model;
	reContextSelectionModel* selectionModel;

	void loadSettings();
	void saveSettings();
	void closeEvent( QCloseEvent * e );
	void newContext();
	void updateToolbar(QToolBar* toolbar);

	virtual bool eventFilter(QObject *object, QEvent *event);

	reViewportWidget *viewset;

	reNode* node();

private:
	Ui::ContextWidget ui;
signals:
	void updateModifier();
public slots:
	void stateChanged();
	void treeContextMenu(QPoint);
	void selectionChanged ( const QItemSelection&, const QItemSelection&);
	void updateMenus();
	void insertActionTriggered();
	void removeObject();
	void save();
	void open( QString path );
	void copy();
	void paste();
	void cut();
	void modifierTriggered();
	void playAnimation();
	void stopAnimation();
	void start(reController* controller=0);
	void pause();
	void stop();
};

#endif // reContextWidget_h__