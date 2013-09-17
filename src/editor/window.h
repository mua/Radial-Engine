#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QItemSelectionModel>

QT_BEGIN_NAMESPACE
class QAction;
class QListWidget;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

class reAsset;
class reAssetsWidget;
class rePropertyWidget;
class rePreviewWidget;
class reSceneGraphWidget;
class reContextWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	QMdiArea *mdiArea;
    MainWindow();
 
private slots:	
	void assetSelectionChanged(reAsset* item);
	void assetDoubleClicked (reAsset* asset);	
	void subWindowActivated ( QMdiSubWindow * window );
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void openContext();
	void newContext();
	void saveContext();
	void saveAsContext();
	void newProject();
	void about();
	void showEvent(QShowEvent * e);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();

	void loadContext(QString fileName);

	void addContextWindow( reContextWindow* context );
	virtual void closeEvent(QCloseEvent *);	

	reContextWindow* _currentWindow;	
	reContextWindow* currentWindow() const;
	void currentWindow(reContextWindow* val);
	void updateCurrentWindow();
	void currentWindowChanged();

	reAssetsWidget* assetsWidget;
	rePropertyWidget* propertyWidget;
	rePreviewWidget* preview;

    QMenu *fileMenu;
    QMenu *editMenu;
	QMenu *insertMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QToolBar *fileToolbar;
    QToolBar *editToolbar;
	QToolBar *modifyToolbar;

    QAction *newProjectAct;
    QAction *saveAct;
	QAction *saveAsAct;
    QAction *openAct;
	QAction *newAct;

    QAction *aboutAct;
    QAction *quitAct;
};
//! [0]

#endif
