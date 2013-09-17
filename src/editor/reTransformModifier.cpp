#include "reTransformModifier.h"

#include "reGizmo.h"
#include "reViewport.h"
#include "reCamera.h"

#include "reContextWindow.h"
#include "reContext.h"

#include "reSelectModifier.h"

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

reTransformModifier::reTransformModifier( QObject* parent ):
	reModifier(parent)
{
	helper = new reNode;
	overNode = 0;
	queryEvent = 0;
}

reTransformModifier::~reTransformModifier()
{
	delete helper;
}

bool reTransformModifier::viewportEventFilter( reViewport* vp, QEvent* event )
{
	/*
	QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event); 
	if (mouseEvent) 
	{
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
			if (mouseEvent->button() == Qt::LeftButton)
			{
				activeViewport = vp;
				overNode = 0;
				queryEvent = new QMouseEvent(*mouseEvent);				
			}
			break;
		
		case QEvent::MouseButtonRelease:
			if (mouseEvent->button() == Qt::LeftButton && overNode && activeViewport == vp)
			{
				overNode->processMessage(&vp->mouseMessageFromQEvent(mouseEvent, reM_MOUSE_RELEASE));
			}
			activeViewport = 0;
			overNode = 0;
			break;
		case QEvent::MouseMove:
			if (overNode && vp == activeViewport)
			{
				overNode->processMessage(&vp->mouseMessageFromQEvent(mouseEvent, reM_MOUSE_MOVE));
			}
			break;
		
		}
	}
	*/
	return false;
}

void reTransformModifier::prerendering( reViewport* vp )
{
	reVec4 p1 = gizmo->worldTransform().matrix * reVec4(0,0,0,1);
	reVec4 p2 = vp->camera->transform.matrix * reVec4(0,0,0,1);
	float s = glm::length(p1-p2) / 80.0f;
	reTransform t = gizmo->transform();	
	t.matrix = glm::scale(reMat4(), reVec3(s, s, s));
	gizmo->transform(t);

	bool handled = false;
	reNodeVector& nodes = ctxWindow->context->selectedObjects();

	if (vp->overNode())
	{
		for (int i=0; i<nodes.size(); i++)
		{
			if (nodes[i] == vp->overNode() || nodes[i]->isChildNode(vp->overNode()))
			{
				handled = true;
				break;
			}
		}
		handled = handled || gizmo->isChildNode(vp->overNode());
	}
	if (handled)
	{
		vp->setCursor(Qt::SizeAllCursor);
	} 
	else
	{
		vp->setCursor(Qt::ArrowCursor);
	}
}

void reTransformModifier::transform( reHelperMoveMessage* message )
{
	qDebug() << message->delta.x << message->delta.y << message->delta.z;
}

void reTransformModifier::attach( reContextWindow* ctxWindow )
{
	reModifier::attach(ctxWindow);

	reVec3 total(0, 0, 0);
	reNodeVector& nodes = ctxWindow->context->selectedObjects();
	for (int i=0; i<nodes.size(); i++)
	{
		reVec4 v(0, 0, 0, 1);
		total += reVec3(nodes[i]->worldTransform().matrix * v);
		observe(nodes[i], reM_MOUSE_PRESS);
		observe(nodes[i], reM_MOUSE_MOVE);
		observe(nodes[i], reM_MOUSE_RELEASE);
	}
	helper->transform(reTransform(glm::translate(reMat4(), total / (float)nodes.size())));
	ctxWindow->helperNode->children->add(helper);
}

void reTransformModifier::detach()
{
	reModifier::detach();
	ctxWindow->helperNode->children->remove(helper);
	reNodeVector& nodes = ctxWindow->context->selectedObjects();
	for (int i=0; i<nodes.size(); i++)
	{
		stopObserving(nodes[i]);
	}
}

void reTransformModifier::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	if (message->id == reM_MOUSE_PRESS)
	{
		ctxWindow->selectModifier->cancel();
	}
	if (gizmo->selected())
	{
		gizmo->selected()->processMessage(message);
	}	
}

void reTransformModifier::transformStarted()
{
	ctxWindow->transformStarted();
}

void reTransformModifier::transformEnded()
{
	ctxWindow->transformEnded();
}

//////////////////////////////////////////////////////////////////////////

reTranslateModifier::reTranslateModifier( QObject* parent ):
	reTransformModifier(parent)
{

}

void reTranslateModifier::attach( reContextWindow* ctxWindow )
{
	reTransformModifier::attach(ctxWindow);
	gizmo = new reTransformGizmo;
	gizmo->modifier = this;
	helper->children->add(gizmo);
}

void reTranslateModifier::detach()
{
	reTransformModifier::detach();
	delete gizmo;
}

void reTranslateModifier::transform( reHelperMoveMessage* message)
{
	if (glm::length(message->delta)>100)
	{
		__debugbreak();
	}
	reTransform t = helper->transform();
	helper->transform(reTransform(glm::translate(t.matrix, message->delta)));

	reNodeVector& nodes = ctxWindow->context->selectedObjects();
	for (int i=0; i<nodes.size(); i++)
	{
		t = nodes[i]->worldTransform();
		nodes[i]->worldTransform(reTransform(glm::translate(reMat4(), message->delta) * t.matrix));
	}
}

//////////////////////////////////////////////////////////////////////////

reScaleModifier::reScaleModifier( QObject* parent ):
	reTransformModifier(parent)
{

}

void reScaleModifier::attach( reContextWindow* ctxWindow )
{
	reTransformModifier::attach(ctxWindow);
	gizmo = new reScaleGizmo;
	gizmo->modifier = this;
	helper->children->add(gizmo);
}

void reScaleModifier::detach()
{
	reTransformModifier::detach();
	delete gizmo;
}

void reScaleModifier::transform( reHelperMoveMessage* message)
{
	if (glm::length(message->delta)>100)
	{
		__debugbreak();
	}

	reVec3 offset = reVec3(helper->worldTransform().matrix * reVec4(0,0,0,1));
	reVec3 delta = message->delta;
	reVec3 origin = message->origin - offset;
	reVec3 scaleVector;
	for (int i=0; i<3; i++)
	{
		scaleVector[i] = origin[i] ? (abs(delta[i]+origin[i]) / abs(origin[i])) : 1;
	}

	reGizmoHelper* gh = (reGizmoHelper*)message->sender;
	if (gh->axes.size()==3)
	{
		scaleVector[0] = scaleVector[2] = scaleVector [1];
	} else if (gh->axes.size()==2)
	{
		float dmax = 0;
		int src = 0;
		for (int i=0; i<3; i++)
		{
			if (abs(scaleVector[i] - 1) > dmax)
			{
				src = i;
				dmax = abs(scaleVector[i] - 1);
			}
		}
		for (int i=0; i<3; i++)
		{				
			scaleVector[i] = scaleVector[i] !=1 ? scaleVector[src] : 1;
			scaleVector[i] = max(0.0f, scaleVector[i]);
		}
	}

	reMat4 scale = glm::scale(reMat4(), scaleVector);

	reTransform t = helper->transform();

	reMat4 om = glm::translate(reMat4(), -reVec3((scale * t.matrix) * reVec4(0,0,0,1) - t.matrix * reVec4(0,0,0,1)));

	helper->transform(reTransform(om * scale * t.matrix));

	reNodeVector& nodes = ctxWindow->context->selectedObjects();
	for (int i=0; i<nodes.size(); i++)
	{
		t = nodes[i]->transform();		
		nodes[i]->transform(reTransform(om * scale * t.matrix));
	}
}

//////////////////////////////////////////////////////////////////////////

reRotateModifier::reRotateModifier( QObject* parent ): reTransformModifier(parent)
{

}

void reRotateModifier::transform( reHelperMoveMessage* message )
{
	reVec3 a = message->origin;
	reVec3 b = message->origin + message->delta;

	reMat4 rotate = 
		glm::rotate(reMat4(), glm::degrees(acos(glm::dot(a, b))), glm::normalize(glm::cross(a, b)));

	for (int i=0; i<16; i++)
	{
		if (glm::isnan(rotate[i%4][i/4])) return;
	}

	reTransform t = helper->transform();

	reMat4 om = glm::translate(reMat4(), -reVec3((rotate * t.matrix) * reVec4(0,0,0,1) - t.matrix * reVec4(0,0,0,1)));

	reNodeVector& nodes = ctxWindow->context->selectedObjects();
	for (int i=0; i<nodes.size(); i++)
	{
		t = nodes[i]->transform();
		nodes[i]->transform(reTransform(om * rotate * t.matrix));
	}
}

void reRotateModifier::attach( reContextWindow* ctxWindow )
{
	reTransformModifier::attach(ctxWindow);
	gizmo = new reRotateGizmo();
	gizmo->modifier = this;
	helper->children->add(gizmo);
}

void reRotateModifier::detach()
{
	reTransformModifier::detach();
	delete gizmo;
}
