#include "reViewport.h"

#include "core.h"
#include <gl/GLU.h>

#include "reCamera.h"
#include "reNode.h"
#include "reRadial.h"
#include "reMessage.h"
#include "reEditor.h"

#include <QTimer>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

reViewport::reViewport(reViewport* shared, QWidget* parent):
	QGLWidget(parent, reEditor::instance()->glWidget)
{
	node = 0;
	helperNode = 0;
	_overNode = 0;

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
	timer->start(1000/60);
	camera = new rePointCamera;
	camera->angles(reVec3(-45, -45, 0));
	camera->distance(300);
	queryEvent = 0;
	boxSelection = false;
	tracking = false;	
	setAutoBufferSwap(false);
	setAutoFillBackground(false);

	setMouseTracking(true);

	//setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	//setMinimumSize(1024, 768);
	//setFixedSize(1024, 768);
}

void reViewport::initializeGL()
{
	qDebug() << "init";
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_MULTISAMPLE);
	glDisable(GL_DITHER);
	/*
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable (GL_POLYGON_SMOOTH);
	glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	*/
}

reNode* reViewport::querySelected(reNode* node)
{
	reEditor::instance()->currentViewport = this;
	QPoint p = mapFromGlobal(QCursor::pos());
	if (p.x()<0 || p.x()>width() || p.y()<0 || p.y()>height()) return 0;
	glClearColor(0,0,0,0);
	glDisable (GL_BLEND);
	glDisable (GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width(), height());
	camera->apply();
	node->render(2);
	GLubyte color[4];
	glReadPixels(p.x(), height()-p.y(), 1, 1, GL_RED, GL_UNSIGNED_BYTE, color);
	return reRadial::shared()->getNode(color[0]);
}

void reViewport::paintGL()
{
	reEditor::instance()->currentViewport = this;
	if (!this->isVisible()) return;
	glClearColor(.15, 0.15, 0.15, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	emit prerendering(this);

	if (!tracking) 
	{
		reNode* onode = querySelected(helperNode);
		if (!onode)
		{
			onode = querySelected(node);
		}
		if (onode != overNode())
		{
			if (overNode())
				overNode()->processMessage(&mouseMessageFromQEvent(reM_MOUSE_LEAVE));
			if (onode)
				onode->processMessage(&mouseMessageFromQEvent(reM_MOUSE_ENTER));
			overNode(onode);
		}
	}

	glViewport(0, 0, width(), height());

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	glEnable ( GL_COLOR_MATERIAL );
	glMatrixMode(GL_MODELVIEW);
	glClearColor(.15, 0.15, 0.15, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	camera->apply();

	glColor4f(0.5, 0.5, 0.5, 1);
	drawGrid(100, 20);
	
	glColor4f(1, 1, 1, 1);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(1);
	drawGrid(50, 40);
	glLineWidth(1);
	glEnable(GL_DEPTH_TEST);

	if (node)
	{
		node->render();
	}	

	QPoint pos = mapFromGlobal(QCursor::pos());
	float mx = pos.x(), my = height() - pos.y();
	glReadPixels(mx, my, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &cursorDepth);
	
	if (helperNode)
	{
		glDisable(GL_CULL_FACE);
		glClear(GL_DEPTH_BUFFER_BIT);
		helperNode->render();
		glEnable(GL_CULL_FACE);
	}

	emit rendering(this);
	swapBuffers();
}

void reViewport::animate()
{
	if (width() && height())
	{
		updateGL();
	}	
};

void reViewport::resizeGL( int w, int h )
{	
	camera->aspect(((float)w)/h);
}

reRay reViewport::pointToRay(QPoint pos)
{
	float mx = pos.x(), my = height() - pos.y();
	double x1, y1, z1;
	double x2, y2, z2;
	GLint viewport[4] = {0, 0, width(), height()};
	glm::dmat4x4 view(camera->view);
	glm::dmat4x4 proj(camera->projection);
	gluUnProject(mx, my, 0.25, glm::value_ptr(view), 
		glm::value_ptr(proj), viewport, &x1, &y1, &z1);
	gluUnProject(mx, my, 0.5, glm::value_ptr(view), 
		glm::value_ptr(proj), viewport, &x2, &y2, &z2);
	reRay ray;
	ray.p = reVec3(x1, y1, z1);
	ray.n = reVec3(x2-x1, y2-y1, z2-z1);
	return ray;
};

reMouseMessage reViewport::mouseMessageFromQEvent(reMessageId id)
{
	QPoint pos = mapFromGlobal(QCursor::pos());
	float mx = pos.x(), my = height() - pos.y();
	double x1, y1, z1;
	double x2, y2, z2;
	GLint viewport[4] = {0, 0, width(), height()};
	glm::dmat4x4 view(camera->view);
	glm::dmat4x4 proj(camera->projection);

	gluUnProject(mx, my, 0.25, glm::value_ptr(view), 
		glm::value_ptr(proj), viewport, &x1, &y1, &z1);
	gluUnProject(mx, my, cursorDepth, glm::value_ptr(view), 
		glm::value_ptr(proj), viewport, &x2, &y2, &z2);
	reMouseMessage mm(id, reVec3(x1, y1, z1), reVec3(x2-x1, y2-y1, z2-z1), QApplication::mouseButtons());
	return mm;
}

void reViewport::mousePressEvent(QMouseEvent *event)
{
	lastPos = event->pos();	
	if (event->buttons() & Qt::LeftButton)
	{
		if (overNode())
		{		
			overNode()->processMessage(&mouseMessageFromQEvent(reM_MOUSE_PRESS));
			tracking = true;
		}		
	}	
}

void reViewport::mouseReleaseEvent( QMouseEvent *event )
{
	if (overNode())
	{		
		overNode()->processMessage(&mouseMessageFromQEvent(reM_MOUSE_RELEASE));
		tracking = false;
	}
	//overNode(0);
}

void reViewport::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - lastPos.x();
	int dy = event->y() - lastPos.y();

	reVec3 angles = camera->angles();
	if (event->buttons() & Qt::RightButton) {
		angles.x += -dy/2.0;
		angles.y += -dx/2.0;
		camera->angles(angles);
	}
	lastPos = event->pos();
	
	//if (event->buttons() & Qt::LeftButton)
	{
		if (overNode())
		{
			overNode()->processMessage(&mouseMessageFromQEvent(reM_MOUSE_MOVE));
		}
	}
}

void reViewport::wheelEvent( QWheelEvent * event )
{
	camera->distance(camera->distance() + pow(abs(camera->distance()), 0.3f) * event->delta() / 25.0f);
}

void reViewport::drawGrid(int count, float size)
{
	glBegin(GL_LINES);
	for (int x=-count/2; x<count/2+1; x++)
	{
		glVertex3f(x*size, 0, -(count*.5f)*size);
		glVertex3f(x*size, 0, (count*.5f)*size);
	}
	for (int z=-count/2; z<count/2+1; z++)
	{
		glVertex3f(-(count*.5f)*size, 0, z*size);
		glVertex3f((count*.5f)*size, 0, z*size);
	}
	glEnd();
}

reNode* reViewport::overNode() const
{
	return _overNode;
}

void reViewport::overNode( reNode* val )
{
	if (_overNode)
	{
		stopObserving(_overNode);
	}
	_overNode = val;
	if (val)
	{
		observe(val, reM_NODE_DESTRUCT);
	}
}

void reViewport::messageProcess( reMessageDispatcher* sender, reMessage* message )
{

	if (message->id == reM_NODE_DESTRUCT && overNode() == sender)
	{
		overNode(0);
	}
}