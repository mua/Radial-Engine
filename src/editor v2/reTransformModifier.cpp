#include "std.h"

#include "reTransformModifier.h"

#include "reGizmo.h"
#include "reViewport.h"
#include "reCamera.h"

#include "reContextWidget.h"
#include "reContextSelectionModel.h"

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
	reNodeVector& nodes = ctxWidget->selectionModel->selectedNodes();

	if (vp->overNode())
	{
		for (unsigned int i =0; i<nodes.size(); i++)
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
		vp->widget->setCursor(Qt::SizeAllCursor);
	} 
}

void reTransformModifier::transform( reHelperMoveMessage* message )
{
	qDebug() << message->delta.x << message->delta.y << message->delta.z;
}

void reTransformModifier::attach( reContextWidget* ctxWindow )
{
	reModifier::attach(ctxWindow);

	reVec3 total(0, 0, 0);
	reNodeVector& nodes = ctxWindow->selectionModel->selectedNodes();
	for (unsigned int i =0; i<nodes.size(); i++)
	{
		reVec4 v(0, 0, 0, 1);
		total += reVec3(nodes[i]->worldTransform().matrix * v);
		observe(nodes[i], reM_MOUSE_PRESS, 0);
		observe(nodes[i], reM_MOUSE_MOVE, 0);
		observe(nodes[i], reM_MOUSE_RELEASE, 0);
	}
	helper->transform(reTransform(glm::translate(reMat4(), total / (float)nodes.size())));
	ctxWindow->helperNode->children->add(helper);
}

void reTransformModifier::detach()
{
	ctxWidget->helperNode->children->remove(helper);	
	reNodeVector& nodes = ctxWidget->selectionModel->selectedNodes();
	for (unsigned int i =0; i<nodes.size(); i++)
	{
		stopObserving(nodes[i]);
	}
	reModifier::detach();
}

void reTransformModifier::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	/*
	if (message->id == reM_MOUSE_PRESS)
	{
		qDebug() << "cancelled";
	}
	*/
	if (gizmo->selected())
	{
		gizmo->selected()->processMessage(message);
	}	
}

void reTransformModifier::transformStarted()
{
	ctxWidget->selectModifier->enabled(false);
	ctxWidget->model->beginUpdates(ctxWidget->selectionModel->selectedObjects());
	//ctxWidget->transformStarted();
}

void reTransformModifier::transformEnded()
{
	ctxWidget->selectModifier->enabled(true);
	//ctxWidget->transformEnded();
	ctxWidget->model->endUpdates();
}

//////////////////////////////////////////////////////////////////////////

reTranslateModifier::reTranslateModifier( QObject* parent ):
	reTransformModifier(parent)
{

}

void reTranslateModifier::attach( reContextWidget* ctxWindow )
{
	reTransformModifier::attach(ctxWindow);
	gizmo = new reTranslateGizmo;
	gizmo->modifier = this;
	helper->children->add(gizmo);
}

void reTranslateModifier::detach()
{
	reTransformModifier::detach();
	gizmo->modifier = 0;
	delete gizmo;
	gizmo = 0;
}

void reTranslateModifier::transform( reHelperMoveMessage* message)
{
	if (glm::length(message->delta)>150)
	{
		__debugbreak();
	}
	reTransform t = helper->transform();
	helper->transform(reTransform(glm::translate(t.matrix, message->delta)));

	reNodeVector& nodes = ctxWidget->selectionModel->selectedNodes();
	for (unsigned int i =0; i<nodes.size(); i++)
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

void reScaleModifier::attach( reContextWidget* ctxWindow )
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
	for (unsigned int i =0; i<3; i++)
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
		for (unsigned int i =0; i<3; i++)
		{
			if (abs(scaleVector[i] - 1) > dmax)
			{
				src = i;
				dmax = abs(scaleVector[i] - 1);
			}
		}
		for (unsigned int i =0; i<3; i++)
		{				
			scaleVector[i] = scaleVector[i] !=1 ? scaleVector[src] : 1;
			scaleVector[i] = max(0.0f, scaleVector[i]);
		}
	}

	reMat4 scale = glm::scale(reMat4(), scaleVector);

	reTransform t = helper->transform();

	reMat4 om = glm::translate(reMat4(), -reVec3((scale * t.matrix) * reVec4(0,0,0,1) - t.matrix * reVec4(0,0,0,1)));

	helper->transform(reTransform(om * scale * t.matrix));

	reNodeVector& nodes = ctxWidget->selectionModel->selectedNodes();
	for (unsigned int i =0; i<nodes.size(); i++)
	{
		t = nodes[i]->worldTransform();		
		nodes[i]->worldTransform(reTransform(om * scale * t.matrix));
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

	for (unsigned int i =0; i<16; i++)
	{
		if (glm::isnan(rotate[i%4][i/4])) return;
	}

	reTransform t = helper->transform();

	reMat4 om = glm::translate(reMat4(), -reVec3((rotate * t.matrix) * reVec4(0,0,0,1) - t.matrix * reVec4(0,0,0,1)));

	reNodeVector& nodes = ctxWidget->selectionModel->selectedNodes();
	for (unsigned int i =0; i<nodes.size(); i++)
	{
		t = nodes[i]->worldTransform();
		nodes[i]->worldTransform(reTransform(om * rotate * t.matrix));
	}
}

void reRotateModifier::attach( reContextWidget* ctxWindow )
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
