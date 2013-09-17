#include "reBone.h"

void lineTo2(reVec3& from, reVec3& to, float tickness = 1)
{
	reVec3 line = to - from;
	float len = glm::length(line);
	reVec3 axis = glm::cross(reVec3(1, 0, 0), line);	
	float angle = acos(glm::dot(line, reVec3(1, 0, 0)) / len);
	reMat4 rot = glm::rotate(reMat4(), glm::degrees(angle),  axis);
	reMat4 scale = glm::scale(reMat4(), reVec3(len+tickness/2, tickness, tickness));
	reMat4 trans = glm::translate(reMat4(), from); 
	reMat4 mat = trans * rot * scale;

	glPushMatrix();

	glMultMatrixf(glm::value_ptr(mat));

	glBegin(GL_QUADS);
	glVertex3f(1, 0.5, 0.5); glVertex3f(0, 0.5, 0.5);
	glVertex3f(0, -0.5, 0.5); glVertex3f(1, -0.5, 0.5);
	glVertex3f(1, -0.5, -0.5); glVertex3f(0, -0.5, -0.5);
	glVertex3f(0, 0.5, -0.5); glVertex3f(1, 0.5, -0.5);
	glVertex3f(1, 0.5, -0.5); glVertex3f(0, 0.5, -0.5);
	glVertex3f(0, 0.5, 0.5); glVertex3f(1, 0.5, 0.5);
	glVertex3f(1, -0.5, 0.5); glVertex3f(0, -0.5, 0.5);
	glVertex3f(0, -0.5, -0.5); glVertex3f(1, -0.5, -0.5);
	glVertex3f(1, -0.5, 0.5); glVertex3f(1, -0.5, -0.5);
	glVertex3f(1, 0.5, -0.5); glVertex3f(1, 0.5, 0.5);
	glVertex3f(0, 0.5, 0.5); glVertex3f(0, 0.5, -0.5);
	glVertex3f(0, -0.5, -0.5); glVertex3f(0, -0.5, 0.5);
	glEnd();
	glPopMatrix();
}

reBone::reBone(): reNode()
{
	renderables->add(boneRenderable = new reBoneRenderable);
}

void reBone::render( int state /*= 0*/ )
{
	reVec4 to(0,0,0,1);
	if (parent()->className() == "reBone")
	{
		to = parent()->worldTransform().matrix * reVec4(0,0,0,1);
		to = worldTransform().inverseMatrix() * to;
	}
	//dir = transform().matrix * reVec4(1,1,1,1);
	boneRenderable->to = reVec3(to);
	reNode::render(state);
}

std::string reBone::className()
{
	return "reBone";
}

void reBoneRenderable::draw( int state /*= 0 */, int id /*= 0*/ )
{
	glColor4f(0,1,0,1);
	glDisable(GL_DEPTH_TEST);	
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(to.x, to.y, to.z);
	glEnd();
	glEnable(GL_DEPTH_TEST);
}
