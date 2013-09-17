#include "rePhysics.h"
#include "reRadial.h"
#include "reRenderable.h"
#include "reMesh.h"
#include "reNode.h"
#include "reTypes.h"
#include "rePTerrain.h"
#include "btBulletDynamicsCommon.h"
#include "reBody.h"
#include "reNoise.h"

#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5

//maximum number of objects (and allow user to shoot additional boxes)
#define MAX_PROXIES (ARRAY_SIZE_X*ARRAY_SIZE_Y*ARRAY_SIZE_Z + 1024)

///scaling of the objects (0.1 = 20 centimeter boxes )
#define SCALING .1
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3

static bool materialCombinerCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
{
	//return true;
	if (colObj0Wrap->getCollisionShape()->getShapeType() == 10)
	{
		reVec3 normal = getNormal(reVec4(fromBullet(cp.getPositionWorldOnB()), 1));
		cp.m_normalWorldOnB = -toBullet(normal);
	}
	else if (colObj1Wrap->getCollisionShape()->getShapeType() == 1)
	{
		reVec3 normal = getNormal(reVec4(fromBullet(cp.getPositionWorldOnA()), 1));
		cp.m_normalWorldOnB = toBullet(normal);
	}
	return true;
}

static void tickCallback(btDynamicsWorld *world, btScalar timeStep) 
{
	rePhysics* phy = (rePhysics*)world->getWorldUserInfo();
	for (size_t i = 0; i<phy->bodies.size(); i++)
	{
		phy->bodies[i]->stepCallback(timeStep);
	}
}

rePhysics::rePhysics()
{
	gContactAddedCallback = materialCombinerCallback;
	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new	btCollisionDispatcher(m_collisionConfiguration);
	m_broadphase = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
	m_solver = sol;	
	dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);
	debugDraw.setDebugMode( btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawText);
	dynamicsWorld->setDebugDrawer(&debugDraw);
	dynamicsWorld->setGravity(btVector3(0,-9.8, 0));
	dynamicsWorld->setWorldUserInfo(this);
	dynamicsWorld->setInternalTickCallback(tickCallback, this);
	time = 0;
}

rePhysics::~rePhysics()
{
	clearPhysics();
}

void rePhysics::initPhysics()
{	
	//addSomeBodies();
	collect();
}

void rePhysics::clearPhysics()
{
	 while(bodies.size())
	 {
		 bodies[0]->removeFromWorld();
	 }
	 int i;
	 for (i=dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	 {
		 btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		 btRigidBody* body = btRigidBody::upcast(obj);
		 if (body && body->getMotionState())
		 {
			 delete body->getMotionState();
		 }
		 dynamicsWorld->removeCollisionObject( obj );
		 delete obj;
	 }

	 //delete collision shapes
	 for ( size_t j=0;j<m_collisionShapes.size();j++)
	 {
		 btCollisionShape* shape = m_collisionShapes[j];
		 delete shape;
	 }
	 m_collisionShapes.clear();
}

void rePhysics::step()
{	
	float delta = reRadial::shared()->time() - time;
	time = reRadial::shared()->time();
	//std::cout << delta << std::endl;
	assert(delta >= 0);
	if (delta)
	{
		dynamicsWorld->stepSimulation(1/60.0f);
	}		
}

void rePhysics::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	step();
}

std::string rePhysics::className()
{
	return "rePhysics";
}

void rePhysics::afterAdd()
{

}

void rePhysics::initControl()
{
	clearPhysics();
	initPhysics();	
}

void rePhysics::exitControl()
{
	clearPhysics();
	stopObserving(reRadial::shared());
}

void rePhysics::pauseControl()
{
	stopObserving(reRadial::shared());
}

void rePhysics::runControl()
{
	time = reRadial::shared()->time();
	observe(reRadial::shared(), reM_TIMER);
}

void rePhysics::registerBody( reBody* body )
{
	if (find(bodies.begin(), bodies.end(), body) == bodies.end())
	{
		bodies.push_back(body);
	}
}

void rePhysics::unregisterBody( reBody* body )
{
	bodies.erase(std::find(bodies.begin(), bodies.end(), body));
}

void rePhysics::collect()
{
	std::vector<reBody*> list;
	super->findObjects(list);
	for ( size_t i=0; i<list.size(); i++)
	{
		if (std::find(bodies.begin(), bodies.end(), list[i]) == bodies.end())
			list[i]->addToWorld(this);
	}
}

void rePhysics::addSomeBodies()
{	
	{
	btBoxShape* colShape = new btBoxShape(btVector3(SCALING*100,SCALING,SCALING*100));
	m_collisionShapes.push_back(colShape);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(btTransform::getIdentity());
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0, 0, colShape);
	btRigidBody* body = new btRigidBody(rbInfo);

	body->setCollisionShape(colShape);
	dynamicsWorld->addRigidBody(body);
	}
	return;
	btBoxShape* colShape = new btBoxShape(btVector3(SCALING*1,SCALING*1,SCALING*1));
	m_collisionShapes.push_back(colShape);
	btTransform startTransform;
	startTransform.setIdentity();
	btScalar	mass(1.f);
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0,0,0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass,localInertia);

	float start_x = START_POS_X - ARRAY_SIZE_X/2;
	float start_y = START_POS_Y;
	float start_z = START_POS_Z - ARRAY_SIZE_Z/2;

	for ( size_t k=0;k<ARRAY_SIZE_Y;k++)
	{
		for ( size_t i=0;i<ARRAY_SIZE_X;i++)
		{
			for(int j = 0;j<ARRAY_SIZE_Z;j++)
			{
				startTransform.setOrigin(SCALING*btVector3(
					btScalar(2.0*i + start_x),
					btScalar(20+2.0*k + start_y),
					btScalar(2.0*j + start_z)));

				btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
				btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,0,localInertia);
				btRigidBody* body = new btRigidBody(rbInfo);

				body->setCollisionShape(colShape);
				dynamicsWorld->addRigidBody(body);
			}
		}
	}
}
