#include "reViewset.h"
#include "reViewport.h"

#include <QSplitter>
#include <QBoxLayout>
#include <QSettings>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QToolbar>
#include <QGLWidget>
#include <QAction>
#include <QActionGroup>
#include <QBoxLayout>

#include "reEditor.h"

reViewset::reViewset(QWidget* parent):
QWidget(parent, reRadial::shared()->glWidget)
{	
	centerDragging = false;
	vs1 = new QSplitter(Qt::Vertical, this);
	vs2 = new QSplitter(Qt::Vertical, this);
	hs = new QSplitter(Qt::Horizontal, this);

	hs->addWidget(vs1);
	hs->addWidget(vs2);

	viewports.resize(4);
	 

	vs1->addWidget(wrap(viewports[0] = new reViewport(NULL, this)));
	vs1->addWidget(wrap(viewports[1] = new reViewport(viewports[0], this)));
	vs2->addWidget(wrap(viewports[2] = new reViewport(viewports[0], this)));
	vs2->addWidget(wrap(viewports[3] = new reViewport(viewports[0], this)));

	for (int i=0; i<4; i++) 
	{
		connect(viewports[i], SIGNAL(nodeClicked(reNode*)), SLOT(nodeClickedVP(reNode*)));
		connect(viewports[i], SIGNAL(nodesSelected(reNodeVector&)), SLOT(nodesSelectedVP(reNodeVector&)));
	}

	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);
	l->addWidget(hs);

	connect(reEditor::instance(), SIGNAL(closing()), SLOT(closing()));

	QSettings settings;
	vs1->restoreState(settings.value("viewsetVs1State").toByteArray());
	vs2->restoreState(settings.value("viewsetVs2State").toByteArray());
	hs->restoreState(settings.value("viewsetHsState").toByteArray());
	connect(vs1, SIGNAL(splitterMoved(int, int)), SLOT(splitterMoved1(int, int)));
	connect(vs2, SIGNAL(splitterMoved(int, int)), SLOT(splitterMoved2(int, int)));

	hs->handle(1)->installEventFilter(this);
	hs->handle(1)->setMouseTracking(true);

	hs->show();
	vs1->show();
	vs2->show();
}

void reViewset::closing()
{
	QSettings settings;
	settings.setValue("viewsetVs1State", vs1->saveState());
	settings.setValue("viewsetVs2State", vs2->saveState());
	settings.setValue("viewsetHsState", hs->saveState());
}

void reViewset::splitterMoved1( int pos, int index )
{
	vs2->setSizes(vs1->sizes());
}

void reViewset::splitterMoved2( int pos, int index )
{
	vs1->setSizes(vs2->sizes());
}

bool reViewset::eventFilter(QObject *obj, QEvent *event)
{	
	QMouseEvent *me = dynamic_cast<QMouseEvent*>(event);
	if (me || (event->type() == QEvent::Leave) || (event->type() == QEvent::Enter) || (event->type() == QEvent::Move) )
	{
		QPoint p = QCursor::pos();
		QPoint vp = vs1->handle(1)->mapToGlobal(QPoint(0, 0));
		bool aroundCenter = (abs(vp.y() - p.y()) <= vs1->handleWidth()*2);
		centerDragging = centerDragging || (aroundCenter && event->type() == QEvent::MouseButtonPress);
		centerDragging = centerDragging && (event->type() != QEvent::MouseButtonRelease);
		if (aroundCenter || centerDragging)
		{
			hs->handle(1)->setCursor(Qt::SizeAllCursor);
		}
		else 
		{
			hs->handle(1)->setCursor(Qt::SplitHCursor);
		}
		if (me && centerDragging)
		{			
			QMouseEvent e2 = QMouseEvent(me->type(), vs1->handle(1)->mapFromGlobal(me->globalPos()), 
				me->globalPos(), me->button(), me->buttons(), me->modifiers());
			QApplication::sendEvent(vs1->handle(1), &e2);			
		}		
	}
	return QWidget::eventFilter(obj, event);
}

reNode* reViewset::node() const
{
	return _node;
}

void reViewset::node( reNode* val )
{
	_node = val;
	for (int i=0; i<4; i++)
	{
		viewports[i]->node = val;
	}
}

void reViewset::nodeClickedVP( reNode *node)
{
	emit nodeClicked(node);
}

reNode* reViewset::helperNode() const
{
	return _helperNode;
}

void reViewset::helperNode( reNode* val )
{
	_helperNode = val;
	for (int i=0; i<4; i++)
	{
		viewports[i]->helperNode = val;
	}
}

void reViewset::nodesSelectedVP(reNodeVector& nodes )
{
	emit nodesSelected(nodes);
}

QWidget * reViewset::wrap( QWidget * wrapped )
{
	return wrapped;
	QWidget *wrapper = new QWidget(this);	
	//wrapper->setLayout(new QHBoxLayout(this));
	//wrapper->layout()->addWidget(wrapped);
	//vs1->addWidget(wrapper);	
	/*
	QSizePolicy sizePolicy4(QSizePolicy::Ignored, QSizePolicy::Ignored);
	sizePolicy4.setHorizontalStretch(0);
	sizePolicy4.setVerticalStretch(0);
	sizePolicy4.setHeightForWidth(wrapped->sizePolicy().hasHeightForWidth());
	wrapped->setSizePolicy(sizePolicy4);
	*/
	wrapped->setParent(wrapper);	
	return wrapper;
}

void reViewset::resizeEvent( QResizeEvent *e )
{
	/*
	for (int i=0; i<viewports.size(); i++)
	{
		if (viewports[i]->geometry().width() < width() || viewports[i]->geometry().height() < height())
		{
			viewports[i]->setGeometry(QRect(0, 0, width() + rand()%25+200, height() + rand()%25+200));
		}
	}
	*/
}