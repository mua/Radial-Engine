#include "reRenderable.h"
#include "core.h"
#include "reShader.h"

reBBox reBBoxRenderable::bBox() const
{
	return _bBox;
}

void reBBoxRenderable::bBox( reBBox val )
{
	_bBox = val;
}
/*
void drawQuad(reVec3 a, reVec3 b) 
{
	glVertex3f(a[0], a[1], a[2]);
	glVertex3f(b[0], a[1], a[2]);

	glVertex3f(b[0], a[1], a[2]);
	glVertex3f(b[0], b[1], a[2]);

	glVertex3f(b[0], b[1], a[2]);
	glVertex3f(b[0], b[1], b[2]);

	glVertex3f(b[0], b[1], b[2]);
	glVertex3f(a[0], a[1], a[2]);
}

void reBBoxRenderable::draw( int state, int id)
{
	reVec3& a = bBox().pMin;
	reVec3& b = bBox().pMax;

	glBegin(GL_LINES);
	glScalef(1.1f, 1.1f, 1.1f);
	drawQuad(reVec3(a[0], a[1], a[2]), reVec3(b[0], b[1], a[2]));
	drawQuad(reVec3(a[0], a[1], b[2]), reVec3(b[0], b[1], b[2]));
	drawQuad(reVec3(a[0], a[1], a[2]), reVec3(a[0], b[1], b[2]));
	drawQuad(reVec3(b[0], a[1], a[2]), reVec3(b[0], b[1], b[2]));
	drawQuad(reVec3(a[0], a[1], a[2]), reVec3(b[0], a[1], b[2]));
	drawQuad(reVec3(a[0], b[1], a[2]), reVec3(b[0], b[1], b[2]));
	glEnd();
}
*/
std::string reRenderable::className()
{
	return "reRenderable";
}

void reRenderable::updateShader( reShader* shader )
{

}

reRenderable::reRenderable()
{
	isTransparent = false;
}

void reRenderable::getTasks( reRenderTask& parent, reTaskList& taskList )
{
	reRenderTask task(parent);
	task.renderable = this;
	taskList.push_back(task);
}

reBBox reRenderable::bBox() const
{
	return _bBox;
}

void reRenderable::bBox( reBBox val )
{
	_bBox = val;
}
