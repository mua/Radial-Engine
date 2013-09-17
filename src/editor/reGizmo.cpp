#include "reGizmo.h"
#include "core.h"

#include <QDebug>
#include <algorithm>
#include <cmath>

#include "reMessage.h"

#include "reTransformModifier.h"

int reTransformGizmo::defaultSelectedIndex = 4;
int reScaleGizmo::defaultSelectedIndex = 4;
int reRotateGizmo::defaultSelectedIndex = 1;

void lineTo(reVec3& from, reVec3& to, float tickness = 1)
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

void cone(float top, int count = 12, float r = .5)
{
	glBegin(GL_TRIANGLES);
	for (int i=0; i<count; i++) 
	{
		float a = i*(M_PI*2.0f/count);
		float b = (i-1)*(M_PI*2.0f/count);
		reVec3 from = reVec3(sin(a), cos(a), 0);
		reVec3 to = reVec3(sin(b), cos(b), 0);		

		glVertex3f(sin(a)*r, cos(a)*r, 0);
		glVertex3f(sin(b)*r, cos(b)*r, 0);
		glVertex3f(0, 0, 0);		
		
		glVertex3f(sin(b)*r, cos(b)*r, 0);
		glVertex3f(sin(a)*r, cos(a)*r, 0);
		glVertex3f(0, 0, top);
	}
	glEnd();
}

void box()
{
	glBegin(GL_QUADS);
	glVertex3f(0.5f, 0.5f, 0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f); glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f); glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f); glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f); glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f); glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f); glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f); glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glEnd();
}

void circle(int count = 16, float radius = 1, float width = 0.05)
{
	for (int i=0; i<count; i++) 
	{
		float a = i*(M_PI*2.0f/count);
		float b = (i+1)*(M_PI*2.0f/count);
		reVec3 from = reVec3(sin(a)*radius, cos(a)*radius, 0);
		reVec3 to = reVec3(sin(b)*radius, cos(b)*radius, 0);
		lineTo(from, to, width);

		/*
		glColor4f(0,1,1, 0.5f);
		glBegin(GL_TRIANGLES);
		glVertex3f(sin(a), cos(a), 0);
		glVertex3f(sin(b), cos(b), 0);
		glVertex3f(0, 0, 0);
		glEnd();
		glColor4f(1,0,0, 1);
		*/
	}
}

reShapeRenderable::reShapeRenderable():
	reRenderable()
{
	color = reColor4(0, 0, 1, 1);
	highlighted = false;
}

void reShapeRenderable::draw( int state /*= 0*/, int id /*= 0*/ )
{
	if (!state)
	{
		if (!highlighted)
		{
			glColor4fv(glm::value_ptr(color));
		}		
		else
		{
			glColor4fv(glm::value_ptr(reVec4(1,1,0,1)));
		}
	}
	drawShape(state, id);
}

void reConeRenderable::drawShape(int state, int id)
{
	cone(2);
}

void reLineRenderable::drawShape(int state, int id)
{
	lineTo(from, to, tickness);
}

reLineRenderable::reLineRenderable():
	reShapeRenderable()
{
	tickness = .2f;
}

void reBoxRenderable::drawShape(int state, int id)
{
	box();
}

void reStripRenderable::drawShape(int state, int id)
{
	if (highlighted || state)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (int i=0; i<points.size(); i++)
		{
			glVertex3fv(glm::value_ptr(points[i]));
		}
		glEnd();
	}
	glColor4f(1, 1, 0, 1);
	glBegin(GL_LINE_STRIP);
	for (int i=0; i<points.size(); i++)
	{
		glVertex3fv(glm::value_ptr(points[i]));
	}
	glEnd();
}

void reCircleRenderable::drawShape( int state /*= 0*/, int id /*= 0*/ )
{
	circle(24, 10, state ? 0.3f : 0.1f );
}

reGizmo::reGizmo():
	reNode()
{
	modifier = 0;
}

bool closestPointOfRayToRay(reVec3& out, reVec3 p1, reVec3 v1, reVec3 p2, reVec3 v2)
{
	reVec3 w = p1 - p2;
	float a = glm::dot(v1, v1);
	float b = glm::dot(v1, v2);
	float c = glm::dot(v2, v2);
	float d = glm::dot(w, v1);
	float e = glm::dot(w, v2);
	if (abs(a*c-b*b) < 0.000001f)
	{
		return false;
	}
	out = p1 + v1 * ((b*e-c*d)/(a*c-b*b));
	return true;
}

bool closestPointOfRayToRay2(reVec3& out, reVec3 p, reVec3 v, reVec3 rp, reVec3 rv)
{
	reVec3 n = glm::cross(glm::cross(v, rv), v);
	reVec3 x = (rp - p);
	float d = glm::dot(x, n);
	float t = d / -glm::dot(n, rv);	
	out = glm::dot(rp + t * rv - p, v) * v + p;
	return true;
}

void reGizmo::processMessage( reMessage* message )
{
	if (reHelperMoveMessage* mm = dynamic_cast<reHelperMoveMessage*>(message))
	{
		if (modifier)
		{
			modifier->transform(mm);
		}
		//reVec3 ip = mm->delta;
		//qDebug() << ip.x << ip.y << ip.z;
		//transform(reTransform(glm::translate(transform().matrix, ip)));
	}
}

reGizmoHelper* reGizmo::selected() const
{
	return _selected;
}

void reGizmo::selected( reGizmoHelper* val )
{
	_selected = val;
}

reGizmoHelper* reGizmo::addHelper (reMat4 transform, reVec3 axis, reShapeRenderable* shape, float distance, bool useLine, reColor4 color, reGizmoHelper* helper)
{
	reGizmoHelper *node = helper ? helper : new reGizmoHelper();
	node->renderables->add(shape);
	if (useLine)
	{
		reLineRenderable* line = new reLineRenderable();
		line->to = reVec3(reVec4(-axis * distance, 1) * transform);
		node->renderables->add(line);
		line->color = color;
	}
	node->transform(reTransform(glm::translate(reMat4(), axis*distance) * transform));
	shape->color = color;
	children->add(node);
	node->axes.push_back(axis);
	node->gizmo = this;
	return node;
};


reGizmoHelper::reGizmoHelper():reNode()
{
	dragging = false;
}

reVec3 reGizmoHelper::axisProjection(reVec3 mp, reVec3 mv)
{
	reVec3 total(0, 0, 0);
	reVec3 origin = reVec3(parent()->worldTransform().matrix * reVec4(0,0,0,1));

	reVec3 out;
	for (reVec3Vector::iterator it = axes.begin(); it != axes.end(); it++)
	{
		out = reVec3(0,0,0); // eksenin world matrix'e gore transform edilmesi lazim.
		if (closestPointOfRayToRay2(out, origin, *it, mp, mv))
			total += (out - origin);
	}
	return total + origin;
};

void reGizmoHelper::processMessage( reMessage* message )
{
	const reMouseMessage* mm = dynamic_cast<const reMouseMessage*>(message);
	if (mm)
	{
		reHelperMoveMessage moveMessage;
		reVec3 p = axisProjection(mm->p, mm->dir);
		switch (mm->id)
		{
		case reM_MOUSE_PRESS:
			dragging = true;
			lastPojection = p;
			gizmo->selected(this);
			gizmo->modifier->transformStarted();
			break;
		case reM_MOUSE_MOVE:
			if (dragging)
			{
				moveMessage.delta = p - lastPojection;
				moveMessage.origin = lastPojection;
				lastPojection = p;
				moveMessage.sender = this;
				reNode::processMessage(&moveMessage);
			}
			break;
		case reM_MOUSE_RELEASE:
			if (dragging)
			{
				gizmo->modifier->transformEnded();
			}
			dragging = false;
			break;
		case reM_MOUSE_ENTER:
			gizmo->highlighted = this;
			break;
		case reM_MOUSE_LEAVE:
			gizmo->highlighted = 0;
			break;
		}
	}
}

void reGizmoHelper::render( int state /*= 0*/ )
{
	for (int i=0; i<renderables->count(); i++)
	{
		((reShapeRenderable*)renderables->at(i))->highlighted = (gizmo->highlighted == this || (!gizmo->highlighted && gizmo->selected() == this));
	}
	reNode::render(state);
}


reTransformGizmo::reTransformGizmo():
	reGizmo()
{
	modifier = 0;
	highlighted = 0;

	addHelper(glm::rotate(reMat4(), 90.0f, reVec3(0, 1, 0)), reVec3(1,0,0), new reConeRenderable(), 5, true, reColor4(.7f,0,0,1));
	addHelper(glm::rotate(reMat4(), -90.0f, reVec3(1, 0, 0)), reVec3(0,1,0), new reConeRenderable(), 5, true, reColor4(0,.7f,0,1));	
	addHelper(reMat4(), reVec3(0,0,1), new reConeRenderable(), 5, true, reColor4(0,0,.7f,1));

	reGizmoHelper* node;
	reConeRenderable* cone;
	reLineRenderable* line;
	reBoxRenderable *box;
	reStripRenderable *strip;

	node = new reGizmoHelper(); // xy
	strip = new reStripRenderable();
	strip->color = reVec4(1,1,0,0.5);
	strip->points.push_back(reVec3(0, 0, 0));
	strip->points.push_back(reVec3(0, 2, 0));
	strip->points.push_back(reVec3(2, 2, 0));
	strip->points.push_back(reVec3(2, 0, 0));
	strip->points.push_back(reVec3(0, 0, 0));
	node->renderables->add(strip);
	children->add(node);
	node->axes.push_back(reVec3(0, 1, 0));
	node->axes.push_back(reVec3(1, 0, 0));
	node->gizmo = this;
	
	node = new reGizmoHelper(); // xz
	strip = new reStripRenderable();
	strip->color = reVec4(1,1,0,0.5);
	strip->points.push_back(reVec3(0, 0, 0));
	strip->points.push_back(reVec3(0, 0, 2));
	strip->points.push_back(reVec3(2, 0, 2));
	strip->points.push_back(reVec3(2, 0, 0));
	strip->points.push_back(reVec3(0, 0, 0));
	node->renderables->add(strip);
	children->add(node);
	node->axes.push_back(reVec3(0, 0, 1));
	node->axes.push_back(reVec3(1, 0, 0));
	node->gizmo = this;

	node = new reGizmoHelper(); // yz
	strip = new reStripRenderable();
	strip->color = reVec4(1,1,0,0.5);
	strip->points.push_back(reVec3(0, 0, 0));
	strip->points.push_back(reVec3(0, 0, 2));
	strip->points.push_back(reVec3(0, 2, 2));
	strip->points.push_back(reVec3(0, 2, 0));
	strip->points.push_back(reVec3(0, 0, 0));
	node->renderables->add(strip);
	children->add(node);
	node->axes.push_back(reVec3(0, 0, 1));
	node->axes.push_back(reVec3(0, 1, 0));
	node->gizmo = this;

	selected((reGizmoHelper*)children->at(defaultSelectedIndex));
}

void reTransformGizmo::selected( reGizmoHelper* val )
{
	reGizmo::selected(val);
	if (val)
	{
		defaultSelectedIndex = children->indexOf(val);
	}
}
//////////////////////////////////////////////////////////////////////////

reScaleGizmo::reScaleGizmo()
{
	modifier = 0;
	highlighted = 0;

	addHelper(glm::rotate(reMat4(), 90.0f, reVec3(0, 1, 0)), reVec3(1,0,0), new reBoxRenderable(), 5, true, reColor4(.7f,0,0,1));
	addHelper(glm::rotate(reMat4(), -90.0f, reVec3(1, 0, 0)), reVec3(0,1,0), new reBoxRenderable(), 5, true, reColor4(0,.7f,0,1));	
	addHelper(reMat4(), reVec3(0,0,1), new reBoxRenderable(), 5, true, reColor4(0,0,.7f,1));

	reGizmoHelper* node;
	reStripRenderable *strip;

	node = new reGizmoHelper(); // xy
	strip = new reStripRenderable();
	strip->color = reVec4(1,1,0,0.5);
	strip->points.push_back(reVec3(2, 0, 0));
	strip->points.push_back(reVec3(4, 0, 0));
	strip->points.push_back(reVec3(0, 4, 0));
	strip->points.push_back(reVec3(0, 2, 0));
	strip->points.push_back(reVec3(2, 0, 0));
	node->renderables->add(strip);
	children->add(node);
	node->axes.push_back(reVec3(0, 1, 0));
	node->axes.push_back(reVec3(1, 0, 0));
	node->gizmo = this;

	node = new reGizmoHelper(); // xz
	strip = new reStripRenderable();
	strip->color = reVec4(1,1,0,0.5);
	strip->points.push_back(reVec3(2, 0, 0));
	strip->points.push_back(reVec3(4, 0, 0));
	strip->points.push_back(reVec3(0, 0, 4));
	strip->points.push_back(reVec3(0, 0, 2));
	strip->points.push_back(reVec3(2, 0, 0));
	node->renderables->add(strip);
	children->add(node);
	node->axes.push_back(reVec3(0, 0, 1));
	node->axes.push_back(reVec3(1, 0, 0));
	node->gizmo = this;

	node = new reGizmoHelper(); // yz
	strip = new reStripRenderable();
	strip->color = reVec4(1,1,0,0.5);
	strip->points.push_back(reVec3(0, 0, 2));
	strip->points.push_back(reVec3(0, 0, 4));
	strip->points.push_back(reVec3(0, 4, 0));
	strip->points.push_back(reVec3(0, 2, 0));
	strip->points.push_back(reVec3(0, 0, 2));
	node->renderables->add(strip);
	children->add(node);
	node->axes.push_back(reVec3(0, 1, 0));
	node->axes.push_back(reVec3(0, 0, 1));
	node->gizmo = this;

	node = new reGizmoHelper(); // xyz
	strip = new reStripRenderable();
	strip->color = reVec4(1,1,0,0.5);
	strip->points.push_back(reVec3(2, 0, 0));
	strip->points.push_back(reVec3(0, 2, 0));
	strip->points.push_back(reVec3(0, 0, 2));
	node->renderables->add(strip);
	children->add(node);
	node->axes.push_back(reVec3(0, 1, 0));
	node->axes.push_back(reVec3(1, 0, 0));
	node->axes.push_back(reVec3(0, 0, 1));
	node->gizmo = this;
	selected((reGizmoHelper*)children->at(defaultSelectedIndex));
}

void reScaleGizmo::selected( reGizmoHelper* val )
{
	reGizmo::selected(val);
	if (val)
	{
		defaultSelectedIndex = children->indexOf(val);
	}
}
//////////////////////////////////////////////////////////////////////////


reVec3 projectPointOntoPlane(reVec3 p, reVec3 n, reVec3 p0)
{
	reVec3 d = p0 - p;
	float t = glm::dot(d, n);
	return (n * -t) + p0;
}

bool planeToDistanceProjection(reVec3& out, reVec3 p, reVec3 v, reVec3 rp, reVec3 rv)
{
	reVec3 n = v;
	reVec3 x = (rp - p);
	float d = glm::dot(x, n);
	float t = d / -glm::dot(n, rv);	
	reVec3 pn = rp + t * rv - p;
	out = glm::normalize(pn);
	/*
	reVec3 ref = projectPointOntoPlane(p, v, reVec3(1,1,1)) - p;
	float cosa = glm::dot(glm::normalize(ref), glm::normalize(pn));

	float angle = -glm::degrees(acos(cosa));
	out = (angle*3) * glm::normalize(v);
	*/
	return true;
}

reVec3 reRotationGizmoHelper::axisProjection( reVec3 mp, reVec3 mv )
{
	reVec3 total(0, 0, 0);
	reVec3 origin = reVec3(parent()->worldTransform().matrix * reVec4(0,0,0,1));

	reVec3 out;
	for (reVec3Vector::iterator it = axes.begin(); it != axes.end(); it++)
	{
		out = reVec3(0,0,0); // eksenin world matrix'e gore transform edilmesi lazim.
		if (planeToDistanceProjection(out, origin, *it, mp, mv))
			total += out;
	}
	return total;
}

reRotateGizmo::reRotateGizmo(): reGizmo()
{
	modifier = 0;
	highlighted = 0;	

	addHelper(glm::rotate(reMat4(), 90.0f, reVec3(0, 1, 0)), reVec3(1,0,0), new reCircleRenderable(), 0, true, reColor4(.7f,0,0,1),  new reRotationGizmoHelper());
	addHelper(glm::rotate(reMat4(), -90.0f, reVec3(1, 0, 0)), reVec3(0,1,0), new reCircleRenderable(), 0, true, reColor4(0,.7f,0,1), new reRotationGizmoHelper());	
	addHelper(reMat4(), reVec3(0,0,1), new reCircleRenderable(), 0, true, reColor4(0,0,.7f,1), new reRotationGizmoHelper());

	selected((reGizmoHelper*)children->at(defaultSelectedIndex));
}

void reRotateGizmo::selected( reGizmoHelper* val )
{
	reGizmo::selected(val);
	if (val)
	{
		defaultSelectedIndex = children->indexOf(val);
	}
}