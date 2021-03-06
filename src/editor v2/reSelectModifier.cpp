#include "std.h"
#include "reSelectModifier.h"

#include "reContextWidget.h"
#include "reContextSelectionModel.h"
#include "reViewportWidget.h"
#include "reViewport.h"

#include "reHelper.h"

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

reSelectModifier::reSelectModifier(QObject* parent):
reModifier(parent)
{
	activeViewport = 0;
	queryNeeded = false;
}

void reSelectModifier::rendering( reViewport* vp )
{
	if (activeViewport == vp)
	{
		QPoint boxEnd = vp->mapFromGlobal(QCursor::pos());

		reVec3 a = reVec3((float)boxStart.x()/vp->size.x, (vp->size.y - boxStart.y())/(float)vp->size.y, -0.9);
		reVec3 b = reVec3((float)boxEnd.x()/vp->size.x, (vp->size.y - boxEnd.y())/(float)vp->size.y, -0.9);

		a = reVec3(a.x*2-1, a.y*2-1, a.z);
		b = reVec3(b.x*2-1, b.y*2-1, b.z);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_CULL_FACE);
		glBegin(GL_QUADS);	
		glColor4f(1,1,0, .3f);
		glVertex3f(a[0], a[1], a[2]);
		glVertex3f(a[0], b[1], a[2]);
		glVertex3f(b[0], b[1], a[2]);
		glVertex3f(b[0], a[1], a[2]);
		glEnd();
		glEnable(GL_CULL_FACE);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	selectionHelper->render();
}

bool reSelectModifier::viewportEventFilter( reViewport* vp, QEvent* event )
{
	QMouseEvent *mouseEvent; 
	switch (event->type())
	{
	case QEvent::MouseButtonPress:
		mouseEvent = (QMouseEvent*)event;
		if (mouseEvent->button() == Qt::LeftButton) {
			if (vp->overNode())
			{
				if (ctxWidget->helperNode->isChildNode(vp->overNode())) 
					return false;
			}
			activeViewport = vp;
			boxStart = vp->mapFromGlobal(QCursor::pos());
		}
		break;
	case QEvent::MouseButtonRelease:
		if (activeViewport)
		{
			selectByBox(boxStart, vp->mapFromGlobal(QCursor::pos()));
		}
		activeViewport = 0;
		break;
	case QEvent::MouseMove:
		break;		
	}		
	return false;
}

reWalkState _walker(reNode* node, void* ctx)
{
	reFrustum *frustum = (reFrustum*)ctx;

	if (node->bBox().isEmpty) return reWS_GO; // PASS olmali aslinda, duzelmek lazim sonra

	reVec3 p[2] = {node->bBox().pMin, node->bBox().pMax};
	bool allIn = true, someIn = false;

	for (unsigned int i =0; i<8; i++)
	{		
		reVec4 v(p[i & 1][0], p[i & 2][1], p[i & 4][2], 1.0f);
		reVec3 w(node->worldTransform().matrix * v);
		bool found = frustum->pointInside(w);
		allIn = allIn && found;
		someIn = someIn || found;
	}
	if (allIn)
	{
		qDebug() << "found:" << node->name().c_str();
	}

	return (reWalkState) (allIn ? (reWS_ADD | reWS_PASS) : reWS_GO);// (someIn ? reWS_GO : reWS_PASS));
}

bool reSelectModifier::selectByBox (QPoint a, QPoint b)
{
	QPoint p1 = a;
	QPoint p2 = b;
	QPoint p3 = QPoint(a.x(), b.y());
	QPoint p4 = QPoint(b.x(), a.y());

	if (abs((a.x()-b.x())*(a.y()-b.y())) < 0.0001f)
	{
		reNodeVector nodes;
		if (activeViewport->overNode() && ctxWidget->node()->isChildNode(activeViewport->overNode()))
		{
			nodes.push_back(activeViewport->overNode());			
		}
		ctxWidget->selectionModel->selectObjects(nodes);
		return false;
	}

	reRay r1 = activeViewport->pointToRay(p1);
	reRay r2 = activeViewport->pointToRay(p2);
	reRay r3 = activeViewport->pointToRay(p3);
	reRay r4 = activeViewport->pointToRay(p4);

	reFrustum frustum;
	frustum.limitless = true;
	frustum.top.p = r1.p;
	frustum.top.n = glm::cross(r1.n, r4.n);
	if (glm::length(frustum.top.n) == 0)
	{
		frustum.top.n = glm::cross(r1.n, r4.p-r1.p);
	}

	frustum.left.p = r1.p;
	frustum.left.n = glm::cross(r1.n, r3.n);
	if (glm::length(frustum.left.n) == 0)
	{
		frustum.left.n = glm::cross(r1.n, r3.p-r1.p);
	}

	frustum.bottom.p = r2.p;
	frustum.bottom.n = glm::cross(r2.n, r3.n);
	if (glm::length(frustum.bottom.n) == 0)
	{
		frustum.bottom.n = glm::cross(r2.n, r3.p-r2.p);
	}

	frustum.right.p = r2.p;
	frustum.right.n = glm::cross(r2.n, r4.n);
	if (glm::length(frustum.right.n) == 0)
	{
		frustum.right.n = glm::cross(r2.n, r4.p-r2.p);
	}

	frustum.normalize();

	reNodeVector nodes;
	ctxWidget->node()->filter(_walker, &frustum, nodes);
	qDebug() << "selected nodes:";
	for (unsigned int i =0; i<nodes.size(); i++)
	{
		qDebug() << nodes[i]->name().c_str();
	}
	ctxWidget->selectionModel->selectObjects(nodes);
	return true;
};

void reSelectModifier::enabled(bool val)
{
	if (!val)
	{
		activeViewport = 0;
	}	
	reModifier::enabled(val);
}

void reSelectModifier::attach( reContextWidget* ctxWidget )
{
	reModifier::attach(ctxWidget);
	selectionHelper = new reSelectionHelper(ctxWidget->selectionModel);
}
