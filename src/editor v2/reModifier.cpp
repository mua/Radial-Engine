#include "std.h"
#include "reModifier.h"

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

#include "reContextWidget.h"
#include "reViewport.h"
#include "reViewportWidget.h"

#include "reHelper.h"

reModifier::reModifier(QObject* parent):
	QObject(parent)
{
	enabled(true);
}

reModifier::~reModifier()
{

}

bool reModifier::eventFilter( QObject *sender, QEvent *event )
{
	return _enabled && viewportEventFilter((reViewport*)sender, event);
}

void reModifier::attach( reContextWidget* ctxWidget )
{
	this->ctxWidget = ctxWidget;
	for (unsigned int i =0; i<ctxWidget->viewset->viewports.size(); i++)
	{
		reViewport* vp = ctxWidget->viewset->viewports[i];
		vp->installEventFilter(this);
		connect(vp, SIGNAL(rendering(reViewport*)), SLOT(rendering(reViewport*)));
		connect(vp, SIGNAL(prerendering(reViewport*)), SLOT(prerendering(reViewport*)));
	}
}

void reModifier::detach()
{
	ctxWidget->removeEventFilter(this);
	for (unsigned int i =0; i<ctxWidget->viewset->viewports.size(); i++)
	{		
		ctxWidget->viewset->viewports[i]->disconnect(this);
		ctxWidget->viewset->viewports[i]->overNode(0);
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

bool reModifier::enabled() const
{
	return _enabled;
}

void reModifier::enabled( bool val )
{
	_enabled = val;
}