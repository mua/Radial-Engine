#ifndef reHelper_h__
#define reHelper_h__

#include "reNode.h"

class reBBoxRenderable;
class reContextWindow;
class reContextSelectionModel;

class reHelper:
	public reNode
{
	
};

class reSelectionHelper:
	public reHelper
{
private:
	reContextSelectionModel* _selectionModel;
public:
	reSelectionHelper(reContextSelectionModel* model);
	virtual void render(int state = 0);
	void clear();
	void addNode(reNode* node);

	reContextSelectionModel* selectionModel() const { return _selectionModel; }
	void selectionModel(reContextSelectionModel* val) { _selectionModel = val; }
};

#endif // reHelper_h__