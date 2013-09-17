#ifndef reSurfaceNode_h__
#define reSurfaceNode_h__

#include "reNode.h"

class reSurfaceNode: public reNode
{
private:
	reVec2 _size;
public:
	reSurfaceNode();
	virtual void render(int state = 0);
	reVec2 size() const;
	void size(reVec2 val);
};

#endif // reSurfaceNode_h__