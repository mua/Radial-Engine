#ifndef reTransformModifier_h__
#define reTransformModifier_h__

#include "reModifier.h"
#include "reMessage.h"

class reGizmo;
class reHelperMoveMessage;

class reTransformModifier:
	public reModifier, public reMessageObserver
{
	Q_OBJECT
private:
	reViewport* activeViewport; 
	QMouseEvent* queryEvent;
	reNode* overNode;
public:
	reNode* helper;
	reGizmo* gizmo;

	reTransformModifier(QObject* parent);
	~reTransformModifier();

	virtual void prerendering(reViewport* vp);
	virtual bool viewportEventFilter( reViewport* vp, QEvent* event );
	virtual void transform (reHelperMoveMessage* message);
	virtual void transformStarted();
	virtual void transformEnded();
	void attach(reContextWidget* ctxWindow);
	void detach();

	void messageProcess(reMessageDispatcher* sender, reMessage* message);
};

class reTranslateModifier:
	public reTransformModifier
{
public:
	reTranslateModifier(QObject* parent);

	virtual void transform(reHelperMoveMessage* message);
	void attach(reContextWidget* ctxWindow);
	virtual void detach();
};

class reScaleModifier:
	public reTransformModifier
{
public:
	reScaleModifier(QObject* parent);

	virtual void transform(reHelperMoveMessage* message);
	void attach(reContextWidget* ctxWindow);
	virtual void detach();
};

class reRotateModifier:
	public reTransformModifier
{
public:
	reRotateModifier(QObject* parent);

	virtual void transform(reHelperMoveMessage* message);
	void attach(reContextWidget* ctxWindow);
	virtual void detach();
};


#endif // reTransformModifier_h__
