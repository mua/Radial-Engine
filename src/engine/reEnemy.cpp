#include "reEnemy.h"
#include "reBody.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "reRadial.h"
#include "reInput.h"
#include "reAnimationSet.h"

reEnemy::reEnemy()
{
	add(body = new reEnemyBody, true);
	body->node(this);	
}

std::string reEnemy::className()
{
	return "reEnemy";
}

void reEnemy::runControl()
{
	observe(reRadial::shared()->input(), reM_MOUSE_PRESS);
	observe(reRadial::shared(), reM_TIMER);
}

void reEnemy::exitControl()
{
	stopObserving(reRadial::shared()->input());
}

void reEnemy::messageProcess( reMessageDispatcher* sender, reMessage* message )
{	
	switch (message->id)
	{
	case reM_MOUSE_PRESS:
		{
			reMouseMessage* mouseMessage = (reMouseMessage*)message;
			reVec3 dir = glm::normalize(mouseMessage->dir);
			reVec3 p(0,1,0);
			target = mouseMessage->p + mouseMessage->dir*(glm::dot(p, mouseMessage->p)/glm::dot(p, -mouseMessage->dir));
			reVec3 d = (mouseMessage->p.y / -dir.y) * dir + mouseMessage->p;
			assert(glm::length(d-target) < 0.1f);
		}
		break;
	case reM_TIMER:
		{
			reVec4 pos = worldTransform().matrix * reVec4(0,0,0,1);
			reVec3 dir = target - reVec3(pos);
			if (glm::length(dir)>.5f)
			{
				dir = glm::normalize(dir);
				body->btBody->applyCentralForce(toBullet(dir)*1000);
			}
			else
			{
				dir = -fromBullet(body->btBody->getLinearVelocity());
				body->btBody->applyCentralForce(toBullet(dir)*100);				
			}
			reVec3 v = fromBullet(body->btBody->getLinearVelocity());
			reNode* node = (reNode*)children->objectByName("model");
			reAnimator* animator = node->children->findObject<reAnimator>();
			if (glm::length(v) > 0.001f)
			{
				float angle = atan2(v.x, v.z);				
				node->transform(reTransform(glm::rotate(reMat4(), glm::degrees(angle), reVec3(0,1,0))));				
				animator->play((reSequence*)node->objectByName("walk"), .3);
				animator->stopAnimation("idle", .3);
			}
			else
			{
				animator->play((reSequence*)node->objectByName("idle"), .3);
				animator->stopAnimation("walk", .3);
			}
		}
		break;
	}
}

void reEnemy::initControl()
{

}

void reEnemy::pauseControl()
{

}

//////////////////////////////////////////////////////////////////////////

reEnemyBody::reEnemyBody()
{
	maxSpeed = 1.5;
	//btBody->setDamping(.5f, 0);
}

void reEnemyBody::addToWorld( rePhysics* physics )
{
	btCollisionShape* shape = new btCapsuleShape(0.3, 1.2);
	btTransform t; t.setIdentity();
	t.setOrigin(btVector3(0,0.9,0));
	compoundShape->addChildShape(t, shape);
	btBody->setAngularFactor(0);
	reBody::addToWorld(physics);
}

void reEnemyBody::stepCallback( float delta )
{
	btVector3 velocity = btBody->getLinearVelocity();
	btScalar speed = velocity.length();
	if(speed > maxSpeed) {
		velocity *= maxSpeed/speed;
		btBody->setLinearVelocity(velocity);
	}
}

