#ifndef rePlayer_h__
#define rePlayer_h_

#include "reNode.h"
#include "reMessage.h"
#include "reController.h"

class reBody;
class rePointCamera;
class reLight;

class rePlayer: public reNode, public reMessageObserver, public reController
{
public:
	reBody* body;
	rePointCamera* camera;
	reLight* light;
	reNode* model();

	rePlayer(reBody* body = 0);
	~rePlayer();
	virtual string className();
	
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) override;
	virtual void runControl() override;
	virtual void afterLoad() override;
};

#endif // rePlayer_h__
