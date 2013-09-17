#ifndef reEditor_h__
#define reEditor_h__


#include <QObject>
#include <gl/glew.h>
#include <QGLWidget>
#include "reProject.h"

class reViewport;

class reEditor: public QObject
{
	Q_OBJECT
public:
	reViewport* currentViewport;
	QGLWidget *glWidget;
	reEditor();
	~reEditor();

	reProject* _project;
	reProject* project() const;
	void project(reProject* val);

	void shutdown();

	static reEditor *instance();	

signals:
	void projectChanged(reProject*);
	void closing();	
};
#endif // reEditor_h__
