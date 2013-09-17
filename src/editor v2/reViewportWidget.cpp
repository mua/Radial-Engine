#include "std.h"
#include "reViewportWidget.h"
#include "reMaker.h"
#include "reSplitter.h"

#include "reRadial.h"
#include "reCamera.h"
#include "reText.h"
#include "reSurfaceNode.h"
#include "reNode.h"
#include "reContextModel.h"
#include "reViewport.h"
#include "reInput.h"

#include <xml/pugixml.hpp>

#include <QResizeEvent>

#include <iostream>

using namespace pugi;
using namespace std;

//////////////////////////////////////////////////////////////////////////

reViewportWidget::reViewportWidget( QWidget* parent )
	:QGLWidget(parent, reMaker::shared()->glWidget)
{
	makeCurrent();
	GLenum err = glewInit();

	installEventFilter(this);
	_model = 0;
	hSplitter = new reGLSplitter(this, reGLSplitter::Horizontal);
	vSplitter = new reGLSplitter(this, reGLSplitter::Vertical);
	connect(hSplitter, SIGNAL(updated()), SLOT(resizeViewports()));
	connect(vSplitter, SIGNAL(updated()), SLOT(resizeViewports()));
	connect(hSplitter, SIGNAL(enter()), SLOT(splitterHover()));
	connect(vSplitter, SIGNAL(enter()), SLOT(splitterHover()));
	connect(hSplitter, SIGNAL(leave()), SLOT(splitterHover()));
	connect(vSplitter, SIGNAL(leave()), SLOT(splitterHover()));
	QSettings settings;
	hSplitter->pos(settings.value("hSplitter", 0.5f).toFloat());
	vSplitter->pos(settings.value("vSplitter", 0.5f).toFloat());
	for (unsigned int i =0; i<4; i++)
	{
		viewports.push_back(new reViewport(this));
	}
	resizeViewports();

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
	timer->start(1000/60.0f);
	activeViewport = 0;
}

void reViewportWidget::closeEvent( QCloseEvent * e )
{
	QSettings settings;
	settings.setValue("hSplitter", hSplitter->pos());
	settings.setValue("vSplitter", vSplitter->pos());
	saveViewportSettings();

}

void reViewportWidget::initializeGL()
{
	qDebug() << "init";
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_MULTISAMPLE);
	glDisable(GL_DITHER);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void reViewportWidget::paintGL()
{		
	ft = reRadial::shared()->time();
	for (unsigned int i =0; i<viewports.size(); i++)
	{
		viewports[i]->render();
	}
	
	makeCurrent();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, (GLint)width(), (GLint)height());
	glClearColor(.15, 0.15, 0.15, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);
	hSplitter->draw();
	vSplitter->draw();
	glEnable(GL_DEPTH_TEST);

	fps = ceil(1.0f / (reRadial::shared()->time() - ft));	
	ft = reRadial::shared()->time();
}

void reViewportWidget::resizeGL( int w, int h )
{
	resizeViewports();
}

void reViewportWidget::animate()
{
	reRadial::shared()->act();
	updateGL();
}

void reViewportWidget::mousePressEvent( QMouseEvent *event )
{
	if (activeViewport) return;
	for (unsigned int i =0; i<viewports.size(); i++)
	{
		if (viewports[i]->sceneRect().contains(event->pos()))
		{
			activeViewport = viewports[i];
			activeViewport->mousePressEvent(event);
		}
	}
}

void reViewportWidget::mouseReleaseEvent( QMouseEvent *event )
{
	if (activeViewport)
	{
		activeViewport->mouseReleaseEvent(event);
	}	
	activeViewport = 0;
}

void reViewportWidget::mouseMoveEvent( QMouseEvent *event )
{
	if (activeViewport)
	{
		activeViewport->mouseMoveEvent(event);
		return;
	}
	for (unsigned int i =0; i<viewports.size(); i++)
	{
		if (viewports[i]->sceneRect().contains(event->pos()))
		{
			viewports[i]->mouseMoveEvent(event);
		}
	}
}

void reViewportWidget::wheelEvent( QWheelEvent * event )
{
	if (activeViewport)
	{
		activeViewport->wheelEvent(event);
		return;
	}
	for (unsigned int i =0; i<viewports.size(); i++)
	{
		if (viewports[i]->sceneRect().contains(event->pos()))
		{
			viewports[i]->wheelEvent(event);
		}
	}
}

void reViewportWidget::resizeViewports()
{
	float my = hSplitter->pos() * height();
	float mx = vSplitter->pos() * width();

	QRect v[] = 
	{
		QRect(0, 0, mx, my),
		QRect(mx+1, 0, width()-mx, my),
		QRect(0, my+2, mx, height()-my),
		QRect(mx+1, my+2, width()-mx, height()-my)
	};

	for (unsigned int i =0; i<viewports.size(); i++)
	{
		viewports[i]->sceneRect(v[i]);
	}
}

reContextSelectionModel* reViewportWidget::selectionModel() const
{
	return _selectionModel;
}

void reViewportWidget::selectionModel( reContextSelectionModel* val )
{
	_selectionModel = val;
}

reContextModel* reViewportWidget::model() const
{
	return _model;
}

void reViewportWidget::model( reContextModel* val )
{
	_model = val;
	loadViewportSettings();
}

void reViewportWidget::splitterHover()
{
	if (hSplitter->hover() && vSplitter->hover())
	{
		setCursor(Qt::SizeAllCursor);
	} else if (vSplitter->hover())
	{
		setCursor(Qt::SizeHorCursor);
	} else if (hSplitter->hover())
	{
		setCursor(Qt::SizeVerCursor);
	} else
	{
		setCursor(Qt::ArrowCursor);
	}
	qDebug() << "hede";
}

bool reViewportWidget::eventFilter( QObject *obj, QEvent *e )
{
	if (e->type() == QEvent::KeyPress)
	{
		reRadial::shared()->input()->keyPress(((QKeyEvent*)e)->key());
	}
	if (e->type() == QEvent::KeyRelease)
	{
		reRadial::shared()->input()->keyUp(((QKeyEvent*)e)->key());
	}
	return false;
}

void reViewportWidget::loadViewportSettings( )
{
	QSettings settings;
	for (unsigned int i =0; i<4; i++)
	{
		if (settings.contains("vpKind"+QVariant(i).toString()))
			viewports[i]->cameraKind((reViewport::CameraKind)settings.value("vpKind"+QVariant(i).toString(), reViewport::User).toInt());
		if (settings.contains("vpPerspective"+QVariant(i).toString()))
			viewports[i]->perspective(settings.value("vpPerspective"+QVariant(i).toString()).toBool());
		if (settings.contains("vpCameraName"+QVariant(i).toString()))
			viewports[i]->setCamera(settings.value("vpCameraName"+QVariant(i).toString()).toString());
	}
}

void reViewportWidget::saveViewportSettings()
{
	QSettings settings;
	for (unsigned int i =0; i<viewports.size(); i++)
	{
		settings.setValue("vpKind"+QVariant(i).toString(), viewports[i]->cameraKind());
		settings.setValue("vpPerspective"+QVariant(i).toString(), viewports[i]->perspective());
		settings.setValue("vpCameraName"+QVariant(i).toString(), viewports[i]->camera->name().c_str());
	}
}
