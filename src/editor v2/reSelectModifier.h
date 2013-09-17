#ifndef reSelectModifier_h__
#define reSelectModifier_h__

#include "reModifier.h"

class reSelectionHelper;

class reSelectModifier:
	public reModifier
{
	Q_OBJECT
private:
	reViewport* activeViewport; 
	QPoint boxStart;
	bool queryNeeded;
	bool selectByBox (QPoint a, QPoint b);
	reSelectionHelper* selectionHelper;
public:
	reSelectModifier(QObject* parent);
	virtual void rendering( reViewport* vp );
	// void prerendering( reViewport* vp );
	virtual void attach(reContextWidget* ctxWidget);

	virtual bool viewportEventFilter(reViewport* vp, QEvent* event);
	void enabled(bool val);
signals:
	void selectionChanged();
	void nodesSelected(reNodeVector& nodes);	
};
#endif // reSelectModifier_h__