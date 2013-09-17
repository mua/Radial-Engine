#include "reNode.h"

class reSamplesCollection: public reTypedCollection<reNode>
{
	virtual reStringList acceptedClasses() override;
};


class reScatter: public reNode, public reMessageObserver
{
public:
	reSamplesCollection* samples;
	reScatter();

	void scatter();

	virtual void parent(reNode* val);
	virtual std::string className() override;
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) override;	
	virtual void getTasks( reRenderTask& parent, reTaskList& tasks ) override;

	bool rayTest(reRay& ray);
	virtual void afterLoad();
};