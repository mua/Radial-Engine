#include "std.h"
#include "reViewport.h"
#include "reViewportWidget.h"
#include "reContextModel.h"

#include <QMouseEvent>

#include "reRadial.h"
#include "reCamera.h"
#include "reText.h"
#include "reSurfaceNode.h"
#include "reNode.h"
#include "reInput.h"

#include "rePhysics.h"
#include "rePhysicsDebug.h"
#include "btBulletDynamicsCommon.h"

reVec2 pointToGL(QPoint p, float height)
{
	return reVec2(p.x(), height - p.y());
}

reViewport::reViewport(reViewportWidget* widget)
{
	_trackMouse = true;
	viewCamera = camera = new rePointCamera();
	camera->name("Viewport");
	camera->angles(reVec3(-45, -45, 0));
	camera->distance(10);
	this->widget = widget;
	label = new reText();
	label->font("fonts/arial.fnt");
	label->text("Utku'dan selamlar!");
	surface = new reSurfaceNode();
	surface->renderables->add(label);
	
	bottomLabel = new reText();
	bottomLabel->font("fonts/arial.fnt");
	bottomLabel->text("FPS");
	bottomSurface = new reSurfaceNode();
	bottomSurface ->renderables->add(bottomLabel);
	
	observe(surface, reM_MOUSE_PRESS);
	observe(surface, reM_MOUSE_RELEASE);	
	tracking = false;
	perspective(true);

	QAction* act;
	contextMenu = new QMenu();
	act = new QAction("Top", this); act->setData((int)Top); 
	connect(act, SIGNAL(triggered()), SLOT(camKindActionTiggered()));
	contextMenu->addAction(act);
	act = new QAction("Left", this); act->setData((int)Left); 
	connect(act, SIGNAL(triggered()), SLOT(camKindActionTiggered()));
	contextMenu->addAction(act);
	act = new QAction("Bottom", this); act->setData((int)Bottom); 
	connect(act, SIGNAL(triggered()), SLOT(camKindActionTiggered()));
	contextMenu->addAction(act);
	act = new QAction("Right", this); act->setData((int)Right); 
	connect(act, SIGNAL(triggered()), SLOT(camKindActionTiggered()));
	contextMenu->addAction(act);
	contextMenu->addSeparator();
	act = new QAction("Perspective", this); act->setCheckable(true);
	connect(act, SIGNAL(triggered(bool)), SLOT(perspectiveActionTriggered(bool)));
	act->setChecked(perspective());
	contextMenu->addAction(act);	
	cameraMenu = contextMenu->addMenu("Camera");
	_overNode = 0;

	renderer = new reRenderer();
}

reMouseMessage reViewport::mouseMessageFromQEvent(reMessageId id)
{
	QPoint pos = widget->mapFromGlobal(QCursor::pos());
	float mx = pos.x(), my = widget->height() - pos.y();
	double x1, y1, z1;
	double x2, y2, z2;
	GLint viewport[4] = {position.x, position.y, size.x, size.y};
	glm::dmat4x4 view(camera->view);
	glm::dmat4x4 proj(camera->projection);

	gluUnProject(mx, my, 0.25, glm::value_ptr(view), 
		glm::value_ptr(proj), viewport, &x1, &y1, &z1);
	gluUnProject(mx, my, 0.15, glm::value_ptr(view), 
		glm::value_ptr(proj), viewport, &x2, &y2, &z2);
	reMouseMessage mm(id, reVec3(x1, y1, z1), glm::normalize(reVec3(x2-x1, y2-y1, z2-z1)), QApplication::mouseButtons());
	reRadial::shared()->input()->dispatchMessage(&mm);
	return mm;
}

void reViewport::relayMouseEvent(QMouseEvent* event)
{
	QApplication::postEvent(this, new QMouseEvent(event->type(), event->pos(), event->globalPos(), event->button(), event->buttons(), Qt::NoModifier));
}

void reViewport::mousePressEvent(QMouseEvent *event)
{
	relayMouseEvent(event);
	reMouseMessage message = mouseMessageFromQEvent(reM_MOUSE_PRESS);
	lastPos = event->pos();	
	if (event->buttons())
	{
		if (overNode())
		{		
			overNode()->processMessage(&message);
			tracking = true;
		}
	}
}

void reViewport::mouseReleaseEvent( QMouseEvent *event )
{
	relayMouseEvent(event);
	reMouseMessage message = mouseMessageFromQEvent(reM_MOUSE_RELEASE);
	if (overNode())
	{		
		overNode()->processMessage(&message);
		tracking = false;
	}
}

void reViewport::mouseMoveEvent( QMouseEvent *event )
{ 
	relayMouseEvent(event);
	int dx = event->x() - lastPos.x();
	int dy = event->y() - lastPos.y();
	lastPos = event->pos();

	reVec3 angles = camera->angles();
	if (event->buttons() & Qt::RightButton) {
		angles.x += -dy/2.0;
		angles.y += -dx/2.0;
		camera->angles(angles);
		cameraKind(User);
	}
	if (overNode())
	{
		overNode()->processMessage(&mouseMessageFromQEvent(reM_MOUSE_MOVE));
	}
}

void reViewport::wheelEvent( QWheelEvent * event )
{
	camera->distance(camera->distance() + pow(abs(camera->distance()), 0.3f) * event->delta() / 150.0f);
}

reNode* reViewport::querySelected(reNode* node, QPoint *ap)
{
	glPushAttrib(GL_ENABLE_BIT);
	QPoint op = ap ? *ap : widget->mapFromGlobal(QCursor::pos());
	QRect rect = sceneRect();
	QPoint p;
	p.setX(op.x() - rect.left());
	p.setY(op.y() - rect.top());
	if (p.x()<0 || p.x()>rect.right() || p.y()<0 || p.y()>rect.bottom()) return 0;
	glClearColor(0,0,0,0);	
	/*
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_LSB_FIRST, 0);
	glPixelStorei(GL_UNPACK_LSB_FIRST, 0);
	*/
	glDisable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_LIGHTING);
	glDisable (GL_MULTISAMPLE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	camera->apply();
	node->render(2);
	GLubyte color[4];	
	glReadPixels(op.x(), widget->height() - op.y(), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
	glPopAttrib();
	int id = (color[0] << 16) + (color[1] << 8) + color[2];
	return reRadial::shared()->getNode(id);
}

void reViewport::render()
{
	reRadial::shared()->camera = camera;
	if (size.x*size.y<100) return;
	glEnable(GL_SCISSOR_TEST);	
	glScissor(position.x, position.y, size.x, size.y);
	glViewport(position.x, position.y, size.x, size.y);
	camera->viewport(reVec4(position.x, position.y, size.x, size.y));

	if (!tracking && trackMouse())
	{
		QPoint p = widget->mapFromGlobal(QCursor::pos());
		reNode* node = querySelected(surface);				
		assert(!node || node == surface);
		if (!node)
		{
			node = querySelected(widget->helperNode);
		}
		if (!node)
		{
			reNode* anode = dynamic_cast<reNode*>(widget->model()->root());
			node = querySelected(anode);
		}
	
		overNode(node);
		//qDebug() << (node ? node->name().c_str() : "none");
		//qDebug() << node;
	}

	glClearColor(0.30f, 0.42f, 0.64f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	emit prerendering(this);
	camera->apply();
	glPushMatrix();
	//glTranslatef(0,-0.1f, 0);
	glDisable(GL_TEXTURE_2D);
	glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
	//drawGrid(100, .5f);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(1);
	drawGrid(50, 1.0f);
	glLineWidth(1);	
	glEnable(GL_DEPTH_TEST);

	glPushAttrib(GL_ENABLE_BIT);
	renderer->render(camera, (reNode*)widget->model()->root());
	glPopAttrib();

	emit rendering(this);

	glClear(GL_DEPTH_BUFFER_BIT);
	widget->helperNode->render();

	rePhysics* phy = ((reCollection*)widget->model()->root())->findInSupers<rePhysics>();
	
	if (phy)
	{
		phy->dynamicsWorld->debugDrawWorld();
	}

	surface->size(size);
	reMat4 t = glm::translate(reMat4(), reVec3(10, size.y - label->font()->lineHeight - 10, 0));
	reMat4 s = glm::scale(reMat4(), reVec3(0.8, 0.8, 0.8));
	surface->transform(reTransform(t*s));
	surface->render();	
	
	bottomSurface->size(size);
	t = glm::translate(reMat4(), reVec3(10, label->font()->lineHeight - 10, 0));
	s = glm::scale(reMat4(), reVec3(0.8, 0.8, 0.8));
	bottomLabel->text(QString("fps:%1").arg(ceil(widget->fps)).toStdString());
	bottomSurface->transform(reTransform(t*s));
	bottomSurface->render();
	
	glPopMatrix();
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

QRect reViewport::sceneRect() const
{
	return QRect(position.x, (widget->height()-position.y)-size.y, size.x, size.y);
}

void reViewport::sceneRect( QRect val )
{
	position = pointToGL(val.bottomLeft(), widget->height());
	size = reVec2(val.width(), val.height());
	camera->aspect(size.x/size.y);
}

void reViewport::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	if (sender == surface && message->id == reM_MOUSE_RELEASE)
	{
		updateCameraMenu();

		contextMenu->exec(QCursor::pos());
	}
	if (message->id == reM_NODE_DESTRUCT && overNode() == sender)
	{
		overNode(0);
	}
}

bool reViewport::perspective() const
{
	return _perspective;
}

void reViewport::perspective( bool val )
{
	_perspective = val;
	camera->perspective(val);
}

reViewport::CameraKind reViewport::cameraKind() const
{
	return _cameraKind;
}

void reViewport::cameraKind( CameraKind val )
{
	_cameraKind = val;
	switch (val)
	{
	case Top:
		camera->angles(reVec3(-90, 0, 0));
		label->text("Top");
		break;
	case Bottom:
		camera->angles(reVec3(-90, 0, 0));
		label->text("Bottom");
		break;
	case Left:
		camera->angles(reVec3(0, 90, 0));
		label->text("Left");
		break;
	case Right:
		camera->angles(reVec3(0, -90, 0));
		label->text("Right");
		break;
	case User:
		label->text("User");
		break;
	}
}

reNode* reViewport::overNode() const
{
	return _overNode;
}

void reViewport::overNode( reNode* val )
{
	if (val != _overNode)
	{
		if (_overNode)
			_overNode->processMessage(&mouseMessageFromQEvent(reM_MOUSE_LEAVE));
		if (val)
		{
			val->processMessage(&mouseMessageFromQEvent(reM_MOUSE_ENTER));
			observe(val, reM_NODE_DESTRUCT);
		} else 
		{
			widget->setCursor(Qt::ArrowCursor);
		}
		_overNode = val;
	}
}

void reViewport::triggered( QAction * action )
{

}

void reViewport::camKindActionTiggered()
{
	QAction* action = dynamic_cast<QAction*>(sender());
	cameraKind((CameraKind)action->data().toInt());
}

void reViewport::perspectiveActionTriggered( bool checked )
{
	perspective(checked);
}

QPoint reViewport::mapFromGlobal( const QPoint & point )
{
	QPoint ret = widget->mapFromGlobal(point);
	ret.setX(ret.x() - position.x);	
	ret.setY((ret.y() - (widget->height() - (position.y+size.y))));
	return ret;
}

reRay reViewport::pointToRay(QPoint pos)
{
	float mx = pos.x(), my = size.y - pos.y();
	double x1, y1, z1;
	double x2, y2, z2;
	GLint viewport[4] = {0, 0, size.x, size.y};
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

QPoint reViewport::mapFromGlobalGL( const QPoint& point )
{
	QPoint r(mapFromGlobal(point));
	r.setY(size.y - r.y());
	return r;
}

void reViewport::camSelect( QAction* act )
{
	setCamera(act->data().toString());
}

void reViewport::updateCameraMenu()
{
	cameraMenu->clear();
	QActionGroup* group = new QActionGroup(cameraMenu);
	group->setExclusive(true);

	std::vector<reCamera*> sceneCams;
	sceneCams.push_back(viewCamera);
	widget->model()->root()->findObjects(sceneCams);	
	
	for (size_t i=0; i<sceneCams.size(); i++ )
	{
		reCamera* cam = sceneCams[i];
		QAction* act = group->addAction(cam->name().c_str());
		act->setCheckable(true);
		cameraMenu->addAction(act);
		act->setData(cam->name().c_str());
		act->setChecked(cam == camera);
	}
	connect(group, SIGNAL(triggered(QAction*)), SLOT(camSelect(QAction*)));
}

void reViewport::setCamera( QString name )
{
	std::vector<reCamera*> sceneCams;
	sceneCams.push_back(viewCamera);
	widget->model()->root()->findObjects(sceneCams);	

	for (size_t i=0; i<sceneCams.size(); i++ )
	{
		if (sceneCams[i]->name().c_str() == name)
		{
			camera = (rePointCamera*)sceneCams[i];
			return;
		}
	}
}

bool reViewport::trackMouse() const
{
	return _trackMouse;
}

void reViewport::trackMouse( bool val )
{
	_trackMouse = val;
}
