#ifndef reHelper_h__
#define reHelper_h__

#include "reNode.h"

class reBBoxRenderable;
class reContextSelectionModel;

class reHelper:
	public reNode
{
	
};

class reSelectionHelper:
	public reHelper
{
private:
	reContextSelectionModel* model;
public:
	reSelectionHelper(reContextSelectionModel* model);
	virtual void render(int state = 0);
	void clear();
	void addNode(reNode* node);
};

#endif // reHelper_h__