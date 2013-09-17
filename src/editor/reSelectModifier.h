#ifndef reSelectModifier_h__
#define reSelectModifier_h__

#include "reModifier.h"

class reSelectModifier:
	public reModifier
{
	Q_OBJECT
private:
	reViewport* activeViewport; 
	QPoint boxStart;
	bool queryNeeded;
	bool selectByBox (QPoint a, QPoint b);

public:
	reSelectModifier(QObject* parent);
	void rendering( reViewport* vp );
	// void prerendering( reViewport* vp );
	void cancel();

	virtual bool viewportEventFilter(reViewport* vp, QEvent* event);

signals:
	void selectionChanged();
	void nodesSelected(reNodeVector& nodes);
};
#endif // reSelectModifier_h__