#include "rePreviewWidget.h"

#include "reMesh.h"
#include "reNode.h"
#include "core.h"

#include <QtGui>
#include "core.h"
#include <math.h>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

rePreviewWidget::rePreviewWidget(QWidget *parent)
: QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	xRot = 0;
	yRot = 0;
	zRot = 0;
	mesh = 0;
	node = 0;
	qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
	qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
	distance = 100;
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
	timer->start(1000/60);
}

rePreviewWidget::~rePreviewWidget()
{
}

QSize rePreviewWidget::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize rePreviewWidget::sizeHint() const
{
	return QSize(400, 400);
}

static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360 * 16;
	while (angle > 360 * 16)
		angle -= 360 * 16;
}

void rePreviewWidget::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		emit xRotationChanged(angle);
	}
}

void rePreviewWidget::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		emit yRotationChanged(angle);
	}
}

void rePreviewWidget::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		emit zRotationChanged(angle);
	}
}

void rePreviewWidget::initializeGL()
{
	qglClearColor(qtPurple.dark());

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_MULTISAMPLE);

}

void rePreviewWidget::paintGL()
{
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -distance);
	glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
	//glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);

	GLfloat ambientColor[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Color(0.2, 0.2, 0.2)
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	GLfloat lightColor0[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Color (0.5, 0.5, 0.5)
	GLfloat lightPos0[] = {1000.0f, 1000.0f, 1000.0f, 1.0f}; //Positioned at (4, 0, 8)
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

	if (mesh)
	{
		mesh->draw();
	}	
	if (node)
	{
		node->render();
	}	

	glDisable(GL_LIGHTING);
	glCullFace(GL_NONE);
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glPolygonOffset(0.1, 0.1);
	glColor3f(0, 1, 0);
	if (mesh)
	{
		mesh->draw(reRS_WIREFRAME);
	}	
	if (node)
	{
		node->render(reRS_WIREFRAME);
	}	
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glPolygonOffset(0, 0);
	glCullFace(GL_BACK);
}

void rePreviewWidget::resizeGL(int width, int height)
{
	//int side = qMin(width, height);
	//glViewport((width - side) / 2, (height - side) / 2, side, side);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,10000.0f);
}

void rePreviewWidget::mousePressEvent(QMouseEvent *event)
{
	lastPos = event->pos();
}

void rePreviewWidget::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - lastPos.x();
	int dy = event->y() - lastPos.y();

	if (event->buttons() & Qt::LeftButton) {
		setXRotation(xRot + 8 * dy);
		setYRotation(yRot + 8 * dx);
	} else if (event->buttons() & Qt::RightButton) {
		setXRotation(xRot + 8 * dy);
		setZRotation(zRot + 8 * dx);
	}
	lastPos = event->pos();
}

void rePreviewWidget::view( reMesh* mesh )
{
	this->mesh = mesh;
	this->node = 0;
}

void rePreviewWidget::view( reNode* node )
{
	this->node = node;
	this->mesh = 0;
}
void rePreviewWidget::wheelEvent( QWheelEvent * event )
{
	distance += event->delta() / 20.0f;
}

void rePreviewWidget::animate()
{
	updateGL();
}
 