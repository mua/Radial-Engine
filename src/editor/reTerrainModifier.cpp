#include "reTerrainModifier.h"
#include "reTerrainPanel.h"
#include "reTerrainNode.h"
#include "reMapBrush.h"

#include "reSelectModifier.h"
#include "reContextWindow.h"

#include <QDockWidget>
#include <QPainter>

#include "reMapBrush.h"

reTerrainModifier::reTerrainModifier( QObject* parent ):
reModifier(parent)
{
	brush(0);
}

void reTerrainModifier::attach( reContextWindow* ctxWindow )
{
	reModifier::attach(ctxWindow);
	panel = new reTerrainPanel(ctxWindow);
	QDockWidget* dock = new QDockWidget("Terrain", ctxWindow);
	dock->setObjectName("Terrain");
	panel = new reTerrainPanel(ctxWindow);
	panel->modifier(this);
	dock->setWidget(panel);
	ctxWindow->addDockWidget(Qt::RightDockWidgetArea, dock);

	if (ctxWindow->context->selectedObjects().size())
	{
		terrain = dynamic_cast<reTerrainNode*>(ctxWindow->context->selectedObjects()[0]);
		if (terrain)
		{
			observe(terrain, reM_MOUSE_PRESS);
			observe(terrain, reM_MOUSE_MOVE);
			observe(terrain, reM_MOUSE_RELEASE);
		}
	}
}

void reTerrainModifier::detach()
{
	reModifier::detach();
	delete panel->parent();
}

void reTerrainModifier::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	if (message->id == reM_MOUSE_PRESS) 
	{
		ctxWindow->selectModifier->cancel();
	}

	reMouseMessage* mm = dynamic_cast<reMouseMessage*>(message);
	if (mm)
	{
		reVec3 hit = mm->dir + mm->p;
		reVec2 pos = terrain->convertPoint(hit);
		if (brush())
		{
			switch (message->id)
			{
			case reM_MOUSE_PRESS:
				brush()->mousePress(terrain, (reMouseMessage*)message, terrain->convertPoint(hit));
				break;
			case reM_MOUSE_MOVE:
				brush()->mouseMove(terrain, (reMouseMessage*)message, terrain->convertPoint(hit));
				break;
			case reM_MOUSE_RELEASE:
				brush()->mouseRelease(terrain, (reMouseMessage*)message, terrain->convertPoint(hit));
				break;
			}
		}
	}

}  