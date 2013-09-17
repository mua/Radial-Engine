#ifndef reBody_h__
#define reBody_h__

#include "reCollection.h"
#include "reMessage.h"

class reNode;
class btConvexHullShape;
class rePhysics;
class reMesh;
class reRenderable;
class rePTerrainRenderable;
class btCollisionShape;
class btCompoundShape;
class btRigidBody;
class btVector3;
class btTransform;
class reMeshRenderable;

class reCollisionShape: public reObject
{
public:
	reRenderable* renderable;
	btCollisionShape* btShape;
	reCollisionShape();
	virtual std::string className() override;
};

class reMeshCollisionShape: public reCollisionShape
{
public:
	reMeshCollisionShape( reMeshRenderable* mr );
};

class reTerrainCollisionShape: public reCollisionShape
{
public:
	reTerrainCollisionShape(rePTerrainRenderable* renderable);
};

typedef std::vector<reCollisionShape*> reShapeList;

class reBody: public reCollection, public reMessageObserver
{
public:
	enum reBodyKind
	{
		reStatic = 0,
		reKinematic = 1,
		reDynamic = 2
	};
private:	
	reNode* _node;
	reBodyKind _kind;
	float _mass;
public:
	rePhysics* physics;
	btCompoundShape* compoundShape;
	btRigidBody* btBody;
	bool autoAdd;
	
	reShapeList shapes;
	reBody();
	~reBody();

	virtual void stepCallback(float delta);
	void updateInertia();
	virtual std::string className() override;
	reNode* node() const;
	void node(reNode* val);
	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) override;

	void readTransform();
	void writeTransform();

	reBody::reBodyKind kind() const;
	void kind(reBody::reBodyKind val);

	void addRenderable( reRenderable* mesh );
	virtual void afterAdd() override;

	void toJson(reVar& val, int categories = ALL_PROPERTIES);
	void loadJson(reVar& value);

	float mass() const;
	void mass(float val);

	virtual void addToWorld(rePhysics* physics);
	virtual void removeFromWorld();
};

reVec3 fromBullet(const btVector3& vec);
reMat4 fromBullet(const btTransform& transform);
btVector3 toBullet(const reVec3& vec);
btTransform toBullet(const reMat4& mat);

#endif // reBody_h__
