#ifndef reViewportWidget_h__
#define reViewportWidget_h__

#include "std.h"
#include "ui_ViewportWidget.h"

#include <vector>

#include "reTypes.h"
#include "reMessage.h"

class reGLSplitter;
class reViewportWidget;
class rePointCamera;
class reText;
class reSurfaceNode;
class reNode;
class reContextModel;
class reContextSelectionModel;
class reViewport;

typedef std::vector<reViewport*> reViewportList;

class reViewportWidget: public QGLWidget
{
	Q_OBJECT
private:
	reGLSplitter* hSplitter;
	reGLSplitter* vSplitter;
	reContextModel* _model;
	reContextSelectionModel* _selectionModel;
public:
	float fps;
	float ft;
	reNode* helperNode;
	reViewportList viewports;
	reViewport* activeViewport;
	reViewportWidget(QWidget* parent);

	void loadViewportSettings( );

	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);

	bool eventFilter(QObject *obj, QEvent *e);
	void mousePressEvent( QMouseEvent *event );
	void mouseReleaseEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void wheelEvent( QWheelEvent * event );
	void closeEvent( QCloseEvent * e );

	void saveViewportSettings( );

	reContextSelectionModel* selectionModel() const;
	void selectionModel(reContextSelectionModel* val);
	reContextModel* model() const;
	void model(reContextModel* val);

public slots:
	void animate();
	void resizeViewports();	
	void splitterHover();
};

#endif // reViewportWidget_h__