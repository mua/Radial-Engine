#include "reTypes.h"
#include <algorithm>

using namespace std;

/*
reVec2::reVec2( float _x, float _y )
{
	this->x = _x;
	this->y = _y;
}

reVec2::reVec2()
{

}

reVec2::reVec2( const reVec2& vec )
{
	x = vec.x;
	y = vec.y;
}

reVec3::reVec3( float x, float y, float z )
{
	this->x = x;
	this->y = y;
	this->z = z;
}

reVec3::reVec3()
{

}

reColor4::reColor4()
{
	r=g=b=a=0;
}

reColor4::reColor4( float r, float g, float b, float a )
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}
*/

reBBox::reBBox():
	pMax(0.0f, 0.0f, 0.0f), pMin(0.0f, 0.0f, 0.0f), isEmpty(true)
{
	
}

reBBox::reBBox( reVec3 max, reVec3 min ):
	pMax(max), pMin(min), isEmpty(true)
{

}

void reBBox::addPoint( const reVec3& pos )
{
	if (isEmpty)
	{
		pMax = pMin = pos;
		isEmpty = false;
	}
	for ( size_t i=0; i<3; i++)
	{
		pMax[i] = max(pMax[i], pos[i]);
		pMin[i] = min(pMin[i], pos[i]);
	}
}

void reBBox::addBox( const reBBox& box )
{
	if (box.isEmpty)
	{
		return;
	}
	addPoint(box.pMax);
	addPoint(box.pMin);
}

void reBBox::addBox( const reBBox& box, const reMat4& transform )
{
	if (box.isEmpty)
	{
		return;
	}
	reVec3 corners[8];
	box.getCorners(corners);
	for (int i=0; i<8; i++)
	{
		reVec4 v(corners[i].x, corners[i].y, corners[i].z, 1);
		addPoint(reVec3(transform * v));
	}
}

void reBBox::getCorners( reVec3 points[] ) const
{
	for ( size_t i=0; i<8; i++)
	{
		for ( size_t k=0; k<3; k++)
		{
			points[i][k] = (i & (1 << k)) ? pMin[k] : pMax[k];
			assert(pMax[k] >= pMin[k]);
		}
	}
}

reVec3 reBBox::center()
{
	return (pMin + pMax)/2.0f;
}

//////////////////////////////////////////////////////////////////////////

reFrustum::reFrustum()
{
	limitless = false;
}

void reFrustum::normalize()
{
	if (glm::dot(left.n, right.p - left.p) < 0) 
		left.n = -left.n;
	if (glm::dot(right.n, left.p - right.p) < 0) 
		right.n = -right.n;
	if (glm::dot(top.n, bottom.p - top.p) < 0) 
		top.n = -top.n;
	if (glm::dot(bottom.n, top.p - bottom.p) < 0) 
		bottom.n = -bottom.n;
}

bool reFrustum::pointInside(const reVec3& v)
{
	reRay *planes[6] = {&top, &left, &bottom, &right, &start, &end};
	for ( size_t i=0; i<(limitless ? 4 : 6); i++)
	{
		if (glm::dot((v - planes[i]->p), planes[i]->n) <= 0) 
			return false;
	}
	return true;
}

reRay toRay(const float a, const float b, const float c, const float d)
{
	float len = glm::length(reVec3(a, b, c));
	reVec3 normal = reVec3(a, b, c) / len;
	return reRay(normal * -d / len, normal);
}

void reFrustum::fromMatrix( reMat4& m )
{
	float a, b, c, d;
	a = m[0][3] + m[0][0]; // left
	b = m[1][3] + m[1][0];
	c = m[2][3] + m[2][0]; 
	d = m[3][3] + m[3][0];
	left = toRay(a, b, c, d);
	
	a = m[0][3] - m[0][0]; // right
	b = m[1][3] - m[1][0];
	c = m[2][3] - m[2][0]; 
	d = m[3][3] - m[3][0];
	right = toRay(a, b, c, d);

	a = m[0][3] + m[0][1]; // bottom
	b = m[1][3] + m[1][1];
	c = m[2][3] + m[2][1]; 
	d = m[3][3] + m[3][1];
	bottom = toRay(a, b, c, d);

	a = m[0][3] - m[0][1]; // top
	b = m[1][3] - m[1][1];
	c = m[2][3] - m[2][1]; 
	d = m[3][3] - m[3][1];
	top = toRay(a, b, c, d);

	a = m[0][3] + m[0][2]; // near
	b = m[1][3] + m[1][2];
	c = m[2][3] + m[2][2]; 
	d = m[3][3] + m[3][2];
	end = toRay(a, b, c, d);

	a = m[0][3] - m[0][2]; // far
	b = m[1][3] - m[1][2];
	c = m[2][3] - m[2][2]; 
	d = m[3][3] - m[3][2];
	start = toRay(a, b, c, d);
}

int reFrustum::classifyBBox( reBBox& bbox )
{
	reVec3 points[8]; bbox.getCorners(points);
	reRay *planes[6] = {&top, &left, &bottom, &right, &start, &end};
	int outCount;

	bool allIn = true;
	bool allOut;
	for ( size_t n=0; n<6; n++)
	{		
		allOut = true;
		for ( size_t i=0; i<8; i++)
		{
			bool test = glm::dot((points[i] - planes[n]->p), planes[n]->n) > 0;
			allIn = allIn & test;
			allOut = allOut & !test;
			if (!allIn && !allOut) break;
		}
		if (allOut)
		{
			return 0;
		}
	}
	return allIn ? 1 : 2;
}

void reFrustum::transform( const reMat4& mat )
{
	reRay *planes[6] = {&top, &left, &bottom, &right, &start, &end};
	for ( size_t n=0; n<6; n++)
	{
		planes[n]->n = reVec3(mat * reVec4(planes[n]->n, 0));
		planes[n]->p = reVec3(mat * reVec4(planes[n]->p, 1));
	}
}

