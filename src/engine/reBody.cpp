#include "reBody.h"
#include "rePhysics.h"
#include "rePTerrain.h"
#include "reNoise.h"
#include "reMeshRenderable.h"

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "btBulletDynamicsCommon.h"

btTransform toBullet(const reMat4& mat)
{
	btTransform bt;
	bt.setFromOpenGLMatrix(glm::value_ptr(mat));
	return bt;
}

reMat4 fromBullet(const btTransform& transform)
{
	float buffer[16];
	transform.getOpenGLMatrix(buffer);
	return glm::make_mat4x4(buffer);
}

btVector3 toBullet(const reVec3& vec)
{
	return btVector3(vec.x, vec.y, vec.z);
}

reVec3 fromBullet(const btVector3& vec)
{
	return reVec3(vec.x(), vec.y(), vec.z());
}

class reMotionState: public btMotionState
{
public:
	reBody* body;
	virtual void getWorldTransform(btTransform& worldTrans ) const
	{
		if (body->node())
		{
			//if (body->kind() == reBody::reStatic) __debugbreak();
			if (body->kind() == reBody::reDynamic) 
				return;
			reVec3 t, s; reMat4 r;
			body->node()->worldTransform().decompose(t, r, s);
			body->compoundShape->setLocalScaling(btVector3(s.x, s.y, s.z));
			worldTrans = toBullet(body->node()->worldTransform().matrix * glm::inverse(glm::scale(reMat4(), s)));
		}
	}

	virtual void setWorldTransform(const btTransform& worldTrans)
	{
		reVec3 t, s; reMat4 r;
		body->node()->worldTransform().decompose(t, r, s);
		body->node()->worldTransform(fromBullet(worldTrans) * glm::scale(reMat4(), s));

		//body->writeTransform();
	}
};

std::string reCollisionShape::className()
{
	return "reCollisionShape";
}

reCollisionShape::reCollisionShape()
{
	renderable = 0;
}

//////////////////////////////////////////////////////////////////////////

reBody::reBody()
{
	_node = 0;	
	physics = 0;
	_mass = 0;
	autoAdd = true;

	compoundShape = new btCompoundShape();	
	btScalar mass(0.);
	btVector3 localInertia(0,0,0);
	reMotionState* myMotionState = new reMotionState();
	myMotionState->body = this;
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,compoundShape,localInertia);				
	btBody = new btRigidBody(rbInfo);
	kind(reKinematic);
	btBody->setCollisionFlags(btBody->getCollisionFlags()  | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}

reBody::~reBody()
{
	if (physics)
	{
		physics->unregisterBody(this);
	}
}

std::string reBody::className()
{
	return "reBody";
}

reNode* reBody::node() const
{
	return _node;
}

void reBody::node( reNode* val )
{
	if (_node == val)
	{
		return;
	}
	_node = val;
	observe(val, reM_TRANSFORM);
	if (rePhysics *phy = super->findInSupers<rePhysics>())
		addToWorld(phy);
	if (autoAdd)
	{
		observe(val, reM_NODE_ADD_RENDERABLE);
		for (int i=0; i<node()->renderables->count(); i++)
		{
			addRenderable(node()->renderables->at(i));
		}
	}
}

void reBody::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	if (message->id == reM_TRANSFORM && !(physics && physics->started))
	{
		readTransform();
	}
	if (message->id == reM_NODE_ADD_RENDERABLE && !(physics && physics->started))
	{
		addRenderable(((reMessageRenderable*)message)->renderable);
	}
}

void reBody::afterAdd()
{
	node((reNode*)super);
}

void reBody::addRenderable(reRenderable* renderable)
{
	reCollisionShape* shape = 0;

	for ( size_t i=0; i<shapes.size(); i++)
	{
		if (shapes[i]->renderable == renderable) return;
	}

	if (rePTerrainRenderable* pt = dynamic_cast<rePTerrainRenderable*>(renderable))
	{
		shape = new reTerrainCollisionShape(pt);
	} 
	else
	{
		shape = new reMeshCollisionShape((reMeshRenderable*)renderable);
	}
	assert(shape);
	shapes.push_back(shape);
	compoundShape->addChildShape(btTransform::getIdentity(), shape->btShape);
}

reBody::reBodyKind reBody::kind() const
{
	return _kind;
}

void reBody::kind( reBodyKind val )
{
	_kind = val;
	switch (val)
	{
	case reKinematic:
		btBody->setCollisionFlags( btBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT); 
		btBody->setActivationState(DISABLE_DEACTIVATION);
		break;
	case reDynamic:
		btBody->setActivationState(DISABLE_DEACTIVATION);
	case reStatic:
	default:
		btBody->setCollisionFlags( btBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT); 
	} 
}

void reBody::toJson( reVar& val, int categories /*= ALL_PROPERTIES*/ )
{
	reCollection::toJson(val, categories);
	val["kind"] = kind();
	val["mass"] = mass();
}

void reBody::loadJson( reVar& value )
{
	reCollection::loadJson(value);
	if (value.isMember("kind"))
	{
		kind(reBodyKind(value["kind"].asInt()));
	}	
	if (value.isMember("mass"))
	{
		mass(value["mass"].asFloat());
	}
}

float reBody::mass() const
{
	return _mass;
}

void reBody::mass( float val )
{
	_mass = val;
	updateInertia();
}

void reBody::updateInertia()
{
	btVector3 inertia; inertia.setZero();
	compoundShape->calculateLocalInertia(mass(), inertia);
	btBody->setMassProps(mass(), inertia);
}

void reBody::readTransform()
{	
	if (physics && physics->started && kind() == reBody::reDynamic) 
		return;
	reVec3 t, s; reMat4 r;
	node()->worldTransform().decompose(t, r, s);
	compoundShape->setLocalScaling(btVector3(s.x, s.y, s.z));
	btTransform worldTrans = toBullet(node()->worldTransform().matrix * glm::inverse(glm::scale(reMat4(), s)));
	btBody->setWorldTransform(worldTrans);
}

void reBody::writeTransform()
{
	reVec3 t, s; reMat4 r;
	node()->worldTransform().decompose(t, r, s);
	node()->worldTransform(fromBullet(btBody->getWorldTransform()) * glm::scale(reMat4(), s));
}

void reBody::addToWorld( rePhysics* physics )
{
	this->physics = physics;
	physics->registerBody(this);
	updateInertia();
	physics->dynamicsWorld->addRigidBody(btBody);
	readTransform();
}

void reBody::removeFromWorld()
{
	physics->unregisterBody(this);
	physics->dynamicsWorld->removeRigidBody(btBody);	
	btBody->setLinearVelocity(btVector3(0,0,0));
	btBody->setAngularVelocity(btVector3(0,0,0));
	physics = 0;
}

void reBody::stepCallback( float delta )
{

}

//////////////////////////////////////////////////////////////////////////

reMeshCollisionShape::reMeshCollisionShape( reMeshRenderable* mr ):
reCollisionShape()
{
	btConvexHullShape *bs = new btConvexHullShape();
	for ( size_t i=0; i<mr->mesh()->groups.size(); i++)
	{
		for ( size_t n=0; n<mr->mesh()->groups[i]->faces.size(); n++)
		{
			for ( size_t v=0; v<3; v++)
			{
				bs->addPoint(toBullet(mr->mesh()->groups[i]->faces[n].vertices[v].pos));
			}
		}
	}
	btShape = bs;
	renderable = mr;
}

class reProcTerrainShape: public btConcaveShape
{
public:
	reProcTerrainShape()
	{
		m_shapeType = CUSTOM_CONCAVE_SHAPE_TYPE;
	}

	btVector3 m_localScaling;

	virtual void getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const
	{
		btVector3 dims(10000, 10000, 10000);
		aabbMin = -dims;
		aabbMax = dims;
	}

	virtual void processAllTriangles(btTriangleCallback* callback,const btVector3& aabbMin,const btVector3& aabbMax) const
	{
		reVec3 expand(1.5,1.5,1.5);
		reVec3 min = fromBullet(aabbMin)-expand,  max = fromBullet(aabbMax)+expand;

		btVector3 c  = toBullet(calculateVertex(reVec3((min.x+max.x)*0.5, 0, (min.z+max.z)*0.5)));
		btVector3 bl = toBullet(calculateVertex(reVec3(min.x, 0, min.z)));
		btVector3 br = toBullet(calculateVertex(reVec3(min.x, 0, max.z)));
		btVector3 tl = toBullet(calculateVertex(reVec3(max.x, 0, min.z)));
		btVector3 tr = toBullet(calculateVertex(reVec3(max.x, 0, max.z)));

		btVector3 vertices[3];
		vertices[0] = bl;
		vertices[1] = br;
		vertices[2] = c;
		assert(btCross(vertices[1]-vertices[0], vertices[2]-vertices[0]).y()>0);
		callback->processTriangle(vertices, 1, 0);
		vertices[0] = tr;
		vertices[1] = tl;
		vertices[2] = c;
		assert(btCross(vertices[1]-vertices[0], vertices[2]-vertices[0]).y()>0);
		callback->processTriangle(vertices, 2, 0);
		vertices[0] = br;
		vertices[1] = tr;
		vertices[2] = c;
		assert(btCross(vertices[1]-vertices[0], vertices[2]-vertices[0]).y()>0);
		callback->processTriangle(vertices, 3, 0);
		vertices[0] = tl;
		vertices[1] = bl;
		vertices[2] = c;
		assert(btCross(vertices[1]-vertices[0], vertices[2]-vertices[0]).y()>0);
		callback->processTriangle(vertices, 4, 0);
	}

	void calculateLocalInertia(btScalar ,btVector3& inertia) const
	{
		inertia.setValue(btScalar(0.),btScalar(0.),btScalar(0.));
	}

	void setLocalScaling(const btVector3& scaling)
	{
		m_localScaling = scaling;
	}
	const btVector3& getLocalScaling() const
	{
		return m_localScaling;
	}

	virtual const char*	getName()const {return "PROCTERRAIN";}

};

reTerrainCollisionShape::reTerrainCollisionShape( rePTerrainRenderable* pt ):
reCollisionShape()
{
	
	rePTerrain* terrain = (rePTerrain*)pt->node;
	btHeightfieldTerrainShape* hs = new btHeightfieldTerrainShape(terrain->collisionCols+1, terrain->collisionRows+1, terrain->heightmap, 1, -100000, 100000, 1, PHY_FLOAT, false);	
	hs->setLocalScaling(btVector3(terrain->collisionWidth, 1, terrain->collisionHeight));
	btShape = hs;	
	
	//btShape = new reProcTerrainShape();
	renderable = pt;
}