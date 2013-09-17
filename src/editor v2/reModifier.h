#ifndef reModifier_h__
#define reModifier_h__

#include "reNode.h"

#include <QObject>
#include <QWidget>

class reContextWidget;
class reViewport;

class reModifier: 
	public QObject
{
	Q_OBJECT
private:
	bool _active;
	bool _enabled;
public:
	reModifier(QObject* parent);
	virtual ~reModifier();
	reContextWidget* ctxWidget;
	virtual void attach(reContextWidget* ctxWindow);
	virtual void detach();

	virtual bool eventFilter(QObject *sender, QEvent *event);
	virtual bool viewportEventFilter(reViewport* vp, QEvent* event);

	bool enabled() const;
	void enabled(bool val);
public:
	bool active() const;
	void active(bool val);
public slots:
	virtual void rendering(reViewport* vp);
	virtual void prerendering( reViewport* vp );
};

#endif // reModifier_h__