#include "reSurfaceNode.h"

reSurfaceNode::reSurfaceNode()
{

}

void reSurfaceNode::render( int state /*= 0*/ )
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, size().x, 0, size().y, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	reNode::render(state);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

reVec2 reSurfaceNode::size() const
{
	return _size;
}

void reSurfaceNode::size( reVec2 val )
{
	_size = val;
}