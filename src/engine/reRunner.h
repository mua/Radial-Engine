#include "reNode.h"
#include "reController.h"
#include "reMessage.h"

class rePointCamera;

class reRunner: public reNode, public reController, public reMessageObserver
{
public:
	float speed;
	reVec3 direction;
	float time;
	rePointCamera* camera;

	reRunner();
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) override;
	virtual void runControl() override;
	virtual void exitControl() override;
	std::string className() override;
};
