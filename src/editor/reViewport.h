#ifndef reViewport_h__
#define reViewport_h__

#include <QWidget>
#include <gl/glew.h>
#include <QGLWidget>

#include "reMessage.h"
#include "reNode.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

class reCamera;
class rePointCamera;

class reViewport
	:public QGLWidget, public reMessageObserver
{
	Q_OBJECT
private:
	QPoint lastPos;

	bool boxSelection;
	QPoint boxStart;
	float cursorDepth;

	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
public:
	reNode* node;
	reNode* helperNode;
	reNode* _overNode;
	reNode* overNode() const;
	void overNode(reNode* val);
	bool tracking;
	QMouseEvent* queryEvent;

	reViewport(reViewport* shared, QWidget* parent);
	void initializeGL();
	void paintGL();

	reNode* querySelected(reNode* node);

	void drawGrid(int count, float size);
	void resizeGL(int w, int h);
	reMouseMessage mouseMessageFromQEvent(reMessageId id);

	rePointCamera* camera;
	reRay pointToRay(QPoint pos);

	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message);

public slots:
	void animate();
	

signals:
	void nodeClicked(reNode*);	
	void nodesSelected(reNodeVector& nodes);
	void rendering(reViewport* vp);
	void prerendering(reViewport* vp);
};

#endif // reViewport_h__