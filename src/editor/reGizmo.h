#ifndef reGizmo_h__
#define reGizmo_h__

#include "reNode.h"
#include "reRenderable.h"
#include "reMessage.h"

typedef std::vector<reVec3> reVec3Vector;

class reShapeRenderable:
	public reRenderable
{
public:
	bool highlighted;
	reColor4 color;
	reShapeRenderable();
	virtual void draw(int state = 0, int id = 0);
	virtual void drawShape(int state = 0, int id = 0)=0;
};

class reConeRenderable:
	public reShapeRenderable
{
	virtual void drawShape(int state = 0, int id = 0);
};

class reLineRenderable:
	public reShapeRenderable
{
public:
	reVec3 from, to;	
	float tickness;
	reLineRenderable();
	virtual void drawShape(int state = 0, int id = 0);
};

class reBoxRenderable:
	public reShapeRenderable
{	
	virtual void drawShape(int state = 0, int id = 0);
};

class reStripRenderable: 
	public reShapeRenderable
{
public:
	vector<reVec3> points;
	virtual void drawShape(int state = 0, int id = 0);
};

class reCircleRenderable:
	public reShapeRenderable
{
	virtual void drawShape(int state = 0, int id = 0);
};

class reHelperMoveMessage:
	public reMessage
{
public:
	reVec3 delta;
	reVec3 origin;
};

class reGizmo;

class reGizmoHelper:
	public reNode
{
public:
	reGizmo* gizmo;
	virtual reVec3 axisProjection(reVec3 mp, reVec3 mv);
	reVec3Vector axes;
	reVec3 lastPojection;
	bool dragging;	
public:
	reGizmoHelper();
	virtual void processMessage( reMessage* message);
	virtual void render(int state = 0);
};

class reRotationGizmoHelper:
	public reGizmoHelper
{
public:
	virtual reVec3 axisProjection(reVec3 mp, reVec3 mv);
};

class reTransformModifier;

class reGizmo:
	public reNode
{
public:	
	reGizmoHelper* _selected;
public:	
	reGizmoHelper* selected() const;
	virtual void selected(reGizmoHelper* val);
	reGizmoHelper* highlighted;
	reNode *indicator;
	reTransformModifier *modifier;
	reGizmo();
	virtual void processMessage( reMessage* message);
	virtual reGizmoHelper* addHelper (reMat4 transform, reVec3 axis, reShapeRenderable* shape, float distance, bool line, reColor4 color, reGizmoHelper* helper=0);
};

class reTransformGizmo:
	public reGizmo
{
public:
	virtual void selected(reGizmoHelper* val);
	static int defaultSelectedIndex;
	reTransformGizmo();	
};

class reScaleGizmo:
	public reGizmo
{
public:
	virtual void selected(reGizmoHelper* val);
	static int defaultSelectedIndex;
	reScaleGizmo();
};

class reRotateGizmo:
	public reGizmo
{
public:
	virtual void selected(reGizmoHelper* val);
	static int defaultSelectedIndex;
	reRotateGizmo();
};

#endif // reGizmo_h__