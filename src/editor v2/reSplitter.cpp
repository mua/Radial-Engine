#include "std.h"
#include "reViewportWidget.h"
#include "reSplitter.h"

#include <QMouseEvent>

reGLSplitter::reGLSplitter( reViewportWidget* widget, reGLSplitter::Kind kind )
{
	this->kind = kind;
	glWidget = widget;	
	widget->installEventFilter(this);
	_pos = 0.5;
	updateRect();
	_hover = false;
	active = false;
	maxP = 0.9f;
	minP = 0.1f;
}

void reGLSplitter::draw()
{	
	float x1 = (float)rect.left() / glWidget->width();
	float x2 = (float)rect.right() / glWidget->width();
	float y1 = (float)(glWidget->height()-rect.top()) / glWidget->height();
	float y2 = (float)(glWidget->height()-rect.bottom()) / glWidget->height();
	x1 = 2*x1 - 1;
	x2 = 2*x2 - 1;
	y1 = 2*y1 - 1;
	y2 = 2*y2 - 1;
	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1, y2, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x2, y1, 0);
	glEnd();
}

bool reGLSplitter::eventFilter( QObject* watched, QEvent* event )
{
	QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
	switch (event->type())
	{
	case QEvent::MouseButtonPress:
		active = rect.contains(me->pos());
		lastMousePos = me->pos();
		//return active;
		break;
	case QEvent::MouseMove:
		{
			hover(active || rect.contains(me->pos()));
			if (active)
			{
				QPoint delta = me->pos() - lastMousePos;
				lastMousePos = me->pos();
				delta = kind == Vertical ? QPoint(delta.x(), 0) : QPoint(0, delta.y());
				qDebug() << delta;
				_pos += (float)delta.x() / glWidget->width();
				_pos += (float)delta.y() / glWidget->height();
				updateRect();
				emit updated();
				//return false;
				break;
			}
		}
	case QEvent::MouseButtonRelease:
		active = false;
		_pos = pos();
		updateRect();
		break;
	case QEvent::Resize:
		updateRect();
	}
	return false;
}

void reGLSplitter::updateRect()
{
	float pos = this->pos();
	if (kind == Horizontal)
	{
		rect.setCoords(0, pos*glWidget->height()-3, glWidget->width(), (pos)*glWidget->height()+3);
	} else
	{
		rect.setCoords(pos*glWidget->width()-3, 0, (pos)*glWidget->width()+3, glWidget->height());
	}
}

float reGLSplitter::pos() const
{
	if (_pos > maxP) return 1;
	if (_pos < minP) return 0;
	return _pos;
}

void reGLSplitter::pos( float val )
{
	_pos = val;
}

bool reGLSplitter::hover() const
{
	return _hover;
}

void reGLSplitter::hover( bool val )
{
	if (val != _hover)
	{
		_hover = val;
		if (val)
			emit enter();
		else
			emit leave();		
	}	
}