#include "reModifier.h"

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

#include "reContextWindow.h"
#include "reViewset.h"
#include "reViewport.h"

#include "reHelper.h"

reModifier::reModifier(QObject* parent):
	QObject(parent)
{
	
}

reModifier::~reModifier()
{

}

bool reModifier::eventFilter( QObject *sender, QEvent *event )
{
	return viewportEventFilter((reViewport*)sender, event);
}

void reModifier::attach( reContextWindow* ctxWindow )
{
	this->ctxWindow = ctxWindow;
	for (int i=0; i<ctxWindow->viewset->viewports.size(); i++)
	{
		ctxWindow->viewset->viewports[i]->installEventFilter(this);
		connect(ctxWindow->viewset->viewports[i], SIGNAL(rendering(reViewport*)), SLOT(rendering(reViewport*)));
		connect(ctxWindow->viewset->viewports[i], SIGNAL(prerendering(reViewport*)), SLOT(prerendering(reViewport*)));
	}
}

void reModifier::detach()
{
	ctxWindow->removeEventFilter(this);
	for (int i=0; i<ctxWindow->viewset->viewports.size(); i++)
	{		
		ctxWindow->viewset->viewports[i]->disconnect(this);
	}
}

bool reModifier::active() const
{
	return _active;
}

void reModifier::active( bool val )
{
	_active = val;
}

void reModifier::rendering( reViewport* vp )
{
	
}

bool reModifier::viewportEventFilter( reViewport* vp, QEvent* event )
{
	return false;
}

void reModifier::prerendering( reViewport* vp )
{

}
