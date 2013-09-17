#include "reVehicle.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Vehicle/btRaycastVehicle.h"
#include "reRadial.h"
#include "reInput.h"
#include "reCamera.h"
#include "rePTerrain.h"
#include "reBody.h"
#include "reNoise.h"
#include "rePhysics.h"
#include "reLight.h"

float	gEngineForce = 0.f;
float	gBreakingForce = 0.f;
float	maxEngineForce = 1000.f;//this should be engine/velocity dependent
float	maxBreakingForce = 100.f;
float	gVehicleSteering = 0.f;
float	steeringIncrement = 0.04f;
float	steeringClamp = 0.3f;
float	wheelRadius = 0.5f;
float	wheelWidth = 0.4f;
float	wheelFriction = 100;//BT_LARGE_FLOAT;
float	suspensionStiffness = 20.f;
float	suspensionDamping = 2.3f;
float	suspensionCompression = 4.4f;
float	rollInfluence = 0.1f;//1.0f;
int rightIndex = 0;
int upIndex = 1;
int forwardIndex = 2;
btVector3 wheelDirectionCS0(0,-1,0);
btVector3 wheelAxleCS(-1,0,0);
btScalar suspensionRestLength(0.6);
#define CUBE_HALF_EXTENTS 1

class reVehicleBody: public reBody
{
public:
	btRaycastVehicle *vehicle;

	reVehicleBody() 
	{
		vehicle = 0;
	}

	virtual void addToWorld(rePhysics* physics)
	{
		reBody::addToWorld(physics);
		btTransform transform; transform.setIdentity();
		compoundShape->addChildShape(transform, new btBoxShape(btVector3(1.f,0.5f,2.f)));
		kind(reBody::reDynamic);
		mass(400);	

		btCylinderShapeX* wheelShape = new btCylinderShapeX(btVector3(wheelWidth,wheelRadius,wheelRadius));

		btRaycastVehicle::btVehicleTuning	m_tuning;

		btDefaultVehicleRaycaster *m_vehicleRayCaster = new btDefaultVehicleRaycaster(physics->dynamicsWorld);
		vehicle = new btRaycastVehicle(m_tuning,btBody,m_vehicleRayCaster);	
		vehicle->setCoordinateSystem(rightIndex, upIndex, forwardIndex);

		float connectionHeight = .5f;

		bool isFrontWheel=true;
		btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
		vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
		vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
		isFrontWheel = false;
		vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
		vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);

		//choose coordinate system
		for (int i=0;i<vehicle->getNumWheels();i++)
		{
			btWheelInfo& wheel = vehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = suspensionDamping;
			wheel.m_wheelsDampingCompression = suspensionCompression;
			wheel.m_frictionSlip = wheelFriction;
			wheel.m_rollInfluence = rollInfluence;
		}

		gVehicleSteering = 0.f;
		
		btBody->setCenterOfMassTransform(btTransform::getIdentity());
		btBody->setLinearVelocity(btVector3(0,0,0));
		btBody->setAngularVelocity(btVector3(0,0,0));
		
		physics->dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(btBody->getBroadphaseHandle(),physics->dynamicsWorld->getDispatcher());

		vehicle->resetSuspension();
		for (int i=0;i<vehicle->getNumWheels();i++)
		{
			//synchronize the wheels with the (interpolated) chassis worldtransform
			vehicle->updateWheelTransform(i, true);
		}

		updateInertia();
		physics->dynamicsWorld->addVehicle(vehicle);	
		readTransform();

		reVehicle* vn = (reVehicle*)node();
		for (size_t i=0; i<4; i++)
		{
			vn->wheelTransforms[i] =
				vn->children->at(0)->children->at(0)->children->at(0)->children->at(i)->transform().matrix * glm::inverse(fromBullet(vehicle->getWheelInfo(i).m_worldTransform));
		}	
	}

	virtual void removeFromWorld()
	{
		physics->dynamicsWorld->removeVehicle(vehicle);
		reBody::removeFromWorld();
	}

};

reVehicle::reVehicle():
	rePlayer(new reVehicleBody)
{
	vehicleBody = (reVehicleBody*)body;
}

std::string reVehicle::className()
{
	return "reVehicle";
}

void reVehicle::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	reVec3 deltaAngle(0,0,0);
	float rs = .5f;
	switch (message->id)
	{
	case reM_TIMER:
		if (started && vehicleBody->vehicle)
		{
			gEngineForce = 0;
			gBreakingForce = 0.f;
			if (reRadial::shared()->input()->keyStates['W'])
			{			
				gEngineForce = maxEngineForce;
			}
			if (reRadial::shared()->input()->keyStates['S'])
			{
				gBreakingForce = maxBreakingForce; 
			}

			if (reRadial::shared()->input()->keyStates[' '])
			{			
			}
			if (reRadial::shared()->input()->keyStates['A'])
			{
				gVehicleSteering += steeringIncrement;
				if (	gVehicleSteering > steeringClamp)
					gVehicleSteering = steeringClamp;
			}
			else if (reRadial::shared()->input()->keyStates['D'])
			{
				gVehicleSteering -= steeringIncrement;
				if (	gVehicleSteering < -steeringClamp)
					gVehicleSteering = -steeringClamp;
			}
			else
				gVehicleSteering += gVehicleSteering>0 ? -steeringIncrement : steeringIncrement;

			int wheelIndex = 2;
			vehicleBody->vehicle->applyEngineForce(gEngineForce,wheelIndex);
			vehicleBody->vehicle->setBrake(gBreakingForce,wheelIndex);
			wheelIndex = 3;
			vehicleBody->vehicle->applyEngineForce(gEngineForce,wheelIndex);
			vehicleBody->vehicle->setBrake(gBreakingForce,wheelIndex);
			wheelIndex = 0;
			vehicleBody->vehicle->setSteeringValue(gVehicleSteering,wheelIndex);
			wheelIndex = 1;
			vehicleBody->vehicle->setSteeringValue(gVehicleSteering,wheelIndex);

			reVec3 pos = worldTransform().position();
			camera->lookAt(pos);
			light->lookAt(pos);
		
			reVec4 dir = glm::normalize(fromBullet(vehicleBody->btBody->getWorldTransform()) * reVec4(0, 0, -1, 0));
			float yaw = atan2(dir.x, dir.z);
			//float yaw = dir.z > 0 ? glm::atan(dir.x / dir.z) : -M_PI + glm::atan(-dir.x / -dir.z);
			//if (yaw<0) yaw += 2*M_PI;

			float pitch = glm::atan(dir.y / abs(dir.z));
			reVec3 da = reVec3(0, glm::degrees(yaw), 0) - camera->lookAngles();
			camera->lookAngles(camera->lookAngles() + da / 20.0f );

			if (pos.y < calculateVertex(reVec4(pos,1)).y)
			{
				//__debugbreak();
			}			
			int map[] = {1,2,3,0};
			for (int i=0; i<4; i++)
			{
				vehicleBody->vehicle->updateWheelTransform(i, true);
				reTransform tw = fromBullet(vehicleBody->btBody->getWorldTransform().inverse() * vehicleBody->vehicle->getWheelInfo(i).m_worldTransform);	

				reTransform tn = children->at(0)->children->at(0)->children->at(0)->children->at(map[i])->transform();
				reVec3 rot = tw.rotation();
				tn.rotation(reVec3(rot.x, 180+rot.z, -rot.y));
				children->at(0)->children->at(0)->children->at(0)->children->at(map[i])->transform(tn);
			}
			break;
		}
		
	}
	
	/*
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
		//model()->children->at(0)->transform(p);
		reVec3 da = reVec3(0, glm::degrees(yaw), 0) - camera->lookAngles();
		camera->lookAngles(camera->lookAngles() + da / 5.0f );
	}
	*/


}

void reVehicle::runControl()
{
	rePTerrain * terrain = root()->findObject<rePTerrain>();
	terrain->camera = camera;
	terrain->player = this;
}

void reVehicle::afterLoad()
{
	observe(reRadial::shared()->input(), reM_KEY_PRESS);
	observe(reRadial::shared()->input(), reM_KEY_UP);
	observe(reRadial::shared(), reM_TIMER);

}
