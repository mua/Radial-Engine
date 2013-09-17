#include "reEditor.h"
#include "reProject.h"

reEditor* _instance = 0;

reEditor::reEditor()
{
	_project = 0;
	glWidget = new QGLWidget(QGLFormat::defaultFormat());	
	glWidget->makeCurrent();
	glewInit();
}

reEditor::~reEditor () 
{
	delete project();
}

//////////////////////////////////////////////////////////////////////////

reEditor *reEditor::instance()
{
	return _instance ? _instance : (_instance = new reEditor);
}

//////////////////////////////////////////////////////////////////////////

reProject* reEditor::project() const
{
	return _project;
}

void reEditor::project( reProject* val )
{
	if (_project != val)
	{
		emit projectChanged(val);
	}
	_project = val;
}

void reEditor::shutdown()
{
	emit closing();
}