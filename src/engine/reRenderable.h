#ifndef reRenderable_h__
#define reRenderable_h__

#include "reTypes.h"
#include "reCollection.h"
#include "reRenderer.h"

class reNode;

class reRenderable: public reCollection, public reRenderClient
{
private:
	reBBox _bBox;
public:
	bool isTransparent;	
	reNode* node;
	reRenderable();
	virtual void draw(int state = 0, int id = 0) = 0;
	virtual string className();
	virtual void updateShader(reShader* shader) override;
	virtual void getTasks( reRenderTask& task, reTaskList& taskList );

	reBBox bBox() const;
	void bBox(reBBox val);
};

class reBBoxRenderable
{
private:
	reBBox _bBox;
public:	
	reBBox bBox() const;
	void bBox(reBBox val);
	virtual void draw(int state = 0, int id = 0);
};

#endif // reRenderable_h__