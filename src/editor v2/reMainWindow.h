#ifndef reMainWindow_h__
#define reMainWindow_h__

#include "std.h"
#include "ui_MainWindow.h"
#include "rePropertyModel.h"

class reMainWindow: public QMainWindow
{
	Q_OBJECT
private:
	bool loaded;
public:
	reMainWindow();
	void closeEvent(QCloseEvent * e);
	void showEvent(QShowEvent * e);
public slots:
	void newObject();
	void openObject(QString path = "");
	void setProject();
	void subWindowActivated(QMdiSubWindow* sub);	
	void updateProperties();
	//void selectionChanged ( const QItemSelection&, const QItemSelection&);
private:
	Ui::MainWindow ui;
};

#endif // reMainWindow_h__