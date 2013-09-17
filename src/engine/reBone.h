#include "reNode.h"
#include "reRenderable.h"

class reBoneRenderable: public reRenderable
{
public:
	reVec3 to;
	virtual void draw(int state = 0 , int id = 0) override;
};

class reBone: public reNode
{
public:
	reBoneRenderable* boneRenderable;
	reBone();
	virtual void render(int state = 0) override;
	virtual string className() override;
};