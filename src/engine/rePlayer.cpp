#include "rePlayer.h"
#include "reBody.h"
#include "btBulletDynamicsCommon.h"
#include "reRadial.h"
#include "reInput.h"
#include "reCamera.h"
#include "rePTerrain.h"
#include "reNoise.h"
#include "reLight.h"

rePlayer::rePlayer(reBody* body)
{
	this->body = body ? body : new reBody;
	add(body, true);
	add(camera = new rePointCamera, true);
	add(light = new reLight, true);
	light->angles(reVec3(-56,-103,0));
	light->distance(6);
	light->farPlane(1000);
	//light->perspective(false);
	camera->angles(reVec3(-22,131,0));
	camera->distance(10);
	camera->farPlane(1000);
}

rePlayer::~rePlayer()
{

}

std::string rePlayer::className()
{
	return "rePlayer";
}

reMat4 directionalRotate(reVec3 a, reVec3 b)
{
	return glm::rotate(reMat4(), glm::degrees(acos(glm::dot(a, b))), glm::normalize(glm::cross(a, b)));
}

void rePlayer::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	reVec3 deltaAngle(0,0,0);
	float rs = .5f;
	switch (message->id)
	{
	case reM_TIMER:
		if (model())
		{
			reVec3 dirZ = reVec3(model()->worldTransform().matrix * reVec4(0,0,-1, 0));
			reVec3 dirX = reVec3(model()->worldTransform().matrix * reVec4(1,0,0, 0));

			if (reRadial::shared()->input()->keyStates['W'])
			{			
				body->btBody->applyCentralForce(toBullet(dirZ * 50.0f));
			}
			if (reRadial::shared()->input()->keyStates['S'])
			{
				body->btBody->applyCentralForce(toBullet(dirZ * -50.0f));
			}
			if (reRadial::shared()->input()->keyStates[' '])
			{			
				body->btBody->applyCentralForce(toBullet(dirX * 1.0f));
				//reVec4 f = worldTransform().matrix * reVec4(0,-200,0,0);
				//btVector3 bf(f.x, f.y, f.z);
				//body->btBody->applyCentralForce(bf);
			}
			if (reRadial::shared()->input()->keyStates['A'])
			{
				reVec3 v = reVec3(glm::rotate(reMat4(), rs, reVec3(0,1,0)) * reVec4(fromBullet(body->btBody->getLinearVelocity()), 0));
				body->btBody->setLinearVelocity(toBullet(v));								
				model()->transform(glm::rotate(reMat4(), rs, reVec3(0,1,0)) * model()->transform().matrix);
				//body->btBody->applyCentralForce(toBullet(f));
			}
			if (reRadial::shared()->input()->keyStates['D'])
			{
				reVec3 v = reVec3(glm::rotate(reMat4(), -rs, reVec3(0,1,0)) * reVec4(fromBullet(body->btBody->getLinearVelocity()), 0));
				body->btBody->setLinearVelocity(toBullet(v));
				model()->transform(glm::rotate(reMat4(), -rs, reVec3(0,1,0)) * model()->transform().matrix);
				//deltaAngle += reVec3(0,-1,0);
				//model()->transform(glm::rotate(reMat4(), -1.0f, reVec3(0,1,0)) * model()->transform().matrix);
			}
			reVec3 pos = worldTransform().position();
			camera->lookAt(pos);
			light->lookAt(pos);
			if (pos.y < calculateVertex(reVec4(pos,1)).y)
			{
				//__debugbreak();
			}
			break;
		}
	}
	
	if (glm::length(fromBullet(body->btBody->getLinearVelocity())))
	{				
		reVec3 dir = glm::normalize(fromBullet(body->btBody->getLinearVelocity()));
		if (!dir.z)
		{
			return;
		}
		float yaw = dir.z < 0 ? glm::atan(dir.x / dir.z) : M_PI - glm::atan(dir.x / -dir.z);
		float pitch = glm::atan(dir.y / abs(dir.z));
		reMat4 y(glm::rotate(reMat4(), glm::degrees(yaw), reVec3(0,1,0)));
		reMat4 p(glm::rotate(reMat4(), glm::degrees(pitch), reVec3(1,0,0)));		
		model()->children->at(0)->transform(p);
		reVec3 da = reVec3(0, glm::degrees(yaw), 0) - camera->lookAngles();
		camera->lookAngles(camera->lookAngles() + da / 5.0f );
	}
}

void rePlayer::runControl()
{
	rePTerrain * terrain = root()->findObject<rePTerrain>();
	terrain->camera = camera;
	terrain->player = this;
	btTransform t = body->btBody->getWorldTransform();
	t.setOrigin(btVector3(-1195.6633, 8.5251445, -1118.4236));
	//body->btBody->setWorldTransform(t);
	//body->btBody->setLinearVelocity(btVector3(-8.97472, -72.7783, 38.4226)/5);
}

void rePlayer::afterLoad()
{
	btTransform transform; transform.setIdentity();
	body->compoundShape->addChildShape(transform, new btSphereShape(0.7f));

	observe(reRadial::shared()->input(), reM_KEY_PRESS);
	observe(reRadial::shared()->input(), reM_KEY_UP);
	observe(reRadial::shared(), reM_TIMER);
	/*
	btTransform transform; transform.setIdentity();
	transform.setRotation(btQuaternion(0,glm::radians(90.0f),0));
	transform.setOrigin(btVector3(0.3f,0,0));
	body->compoundShape->addChildShape(transform, new btCapsuleShape(.3f, 1.0f));
	transform.setOrigin(btVector3(-0.3f,0,0));
	body->compoundShape->addChildShape(transform, new btCapsuleShape(.3f, 1.0f));
	body->btBody->setCcdMotionThreshold(1e-7f);
	body->btBody->setCcdSweptSphereRadius(0.5f);
	body->btBody->setDamping(0.0f, 1.0f);
	
	body->btBody->setAnisotropicFriction(btVector3(1,0,0));
	*/
	body->btBody->setAngularFactor(0);
	body->btBody->setCcdMotionThreshold(0.01);
	body->btBody->setCcdSweptSphereRadius(0.8f);
	//body->btBody->setCenterOfMassTransform(btTransform::getIdentity());
	//body->btBody->setDamping(0, 100000);
}

reNode* rePlayer::model()
{
	return children->count() ? children->at(0) : 0;
}
