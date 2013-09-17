#include "reNode.h"
#include "reController.h"
#include "reBody.h"
#include "reMessage.h"

class reEnemyBody: public reBody
{
public:
	float maxSpeed;
	reEnemyBody();
	virtual void addToWorld(rePhysics* physics) override;
	virtual void stepCallback(float delta);
};

class reEnemy: public reNode, public reController, public reMessageObserver
{
public:
	reEnemyBody* body;
	reEnemy();
	virtual std::string className() override;
	reVec3 target;

	virtual void initControl();
	virtual void pauseControl();
	virtual void exitControl();
	virtual void runControl();

	void messageProcess( reMessageDispatcher* sender, reMessage* message );
};