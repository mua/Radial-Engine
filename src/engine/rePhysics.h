#ifndef rePhysics_h__
#define rePhysics_h__

#include "reMessage.h"
#include "reObject.h"
#include "reCollection.h"
#include "rePhysicsDebug.h"
#include "reTypes.h"
#include "reController.h"

#include <vector>
#include <string>

class reBody;

typedef vector<reBody*> reBodyVector;

class rePhysics: public reObject, public reMessageObserver, public reController
{
private:
	float time;
	btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;
	btBroadphaseInterface*	m_broadphase;
	btCollisionDispatcher*	m_dispatcher;
	btConstraintSolver*	m_solver;
	btDefaultCollisionConfiguration* m_collisionConfiguration;
public:
	reBodyVector bodies;

	btDiscreteDynamicsWorld* dynamicsWorld;
	GLDebugDrawer debugDraw;
	rePhysics();
	virtual void afterAdd() override;
	virtual ~rePhysics();
	void initPhysics();

	void addSomeBodies();

	void collect();

	void clearPhysics();
	void step();
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) override;
	virtual std::string className() override;

	void registerBody(reBody* body);
	void unregisterBody(reBody* body);

	virtual void initControl();
	virtual void exitControl();
	virtual void runControl();
	virtual void pauseControl();
};

#endif // rePhysics_h__
