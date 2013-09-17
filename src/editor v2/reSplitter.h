#include "std.h"

class reViewportWidget; 

class reGLSplitter: public QObject
{
	Q_OBJECT
private:
	bool _hover;
public:
	typedef enum
	{
		Horizontal,
		Vertical
	} Kind;
	Kind kind;
	bool active;
	QPoint lastMousePos;
	reViewportWidget* glWidget;
	QRect rect;
	float _pos;
	float maxP, minP;
	reGLSplitter(reViewportWidget* widget, reGLSplitter::Kind kind);
	void draw();
	bool eventFilter(QObject* watched, QEvent* event);
	void updateRect();
	float pos() const;
	void pos(float val);
	bool hover() const;
	void hover(bool val);
signals:
	void updated();
	void enter();
	void leave();
};