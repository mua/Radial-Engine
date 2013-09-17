#ifndef reViewport_h__
#define reViewport_h__
#include "std.h"

#include "reMessage.h"

class reViewportWidget;
class reText;
class rePointCamera;
class reSurfaceNode;
class reRenderer;

class reViewport: public QObject, public reMessageObserver
{
	Q_OBJECT
public:
	typedef enum {
		Top,
		Left,
		Right,
		Bottom,
		User
	} CameraKind;
private:
	reRenderer* renderer;
	QRect _sceneRect;
	CameraKind _cameraKind;
	bool _perspective;
	reNode* _overNode;
	bool tracking;
	QMenu *contextMenu;
	QMenu *cameraMenu;
	void relayMouseEvent(QMouseEvent* event);
	bool _trackMouse;
public:
	rePointCamera *camera, *viewCamera;
	reVec2 position;
	reVec2 size;
	QPoint lastPos;
	reViewportWidget* widget;
	reText *label;
	reSurfaceNode* surface;

	reText *bottomLabel;
	reSurfaceNode* bottomSurface;

	reViewport(reViewportWidget* widget);
	void mousePressEvent( QMouseEvent *event );
	void mouseReleaseEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void wheelEvent( QWheelEvent * event );

	void render();
	void drawGrid(int count, float size);

	QRect sceneRect() const;
	void sceneRect(QRect val);
	reNode* querySelected(reNode* node, QPoint *ap = 0);
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message);

	void updateCameraMenu();

	reViewport::CameraKind cameraKind() const;
	void cameraKind(reViewport::CameraKind val);
	void setCamera(QString name);

	bool perspective() const;
	void perspective(bool val);
	reNode* overNode() const;
	void overNode(reNode* val);
	bool trackMouse() const;
	void trackMouse(bool val);

	reMouseMessage mouseMessageFromQEvent(reMessageId id);
	QPoint mapFromGlobal(const QPoint &);
	QPoint mapFromGlobalGL(const QPoint &);
	reRay pointToRay(QPoint pos);
	reNode* querySelected2(reNode* node, QPoint op );
public slots:
	void triggered ( QAction * action );
	void camKindActionTiggered ();
	void perspectiveActionTriggered(bool checked);
	void camSelect( QAction* );
signals:
	void rendering(reViewport*);
	void prerendering(reViewport*);
	

};
#endif // reViewport_h__
