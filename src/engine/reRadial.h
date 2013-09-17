#ifndef engine_h__
#define engine_h__

#include <map>
#include "core.h"
#include "reMessage.h"

class reAssetLoader; 
class reNode;
class reCamera;
class rePhysics;
class reInput;
class reRenderer;

class reRadial: public reMessageDispatcher
{
private:
	reAssetLoader* _assetLoader;
	rePhysics* _physics;
	reInput* _input;
	reRenderer* _renderer;
	std::map<int, reNode*> nodes;
	int counter;
public:
	reCamera* camera;
	reRadial();

	reAssetLoader* assetLoader() const;
	void assetLoader(reAssetLoader* val);
	static reRadial* shared();

	int registerNode(reNode* node);
	void unregisterNode(int handle);
	reNode* getNode(int handle);
	float time();
	void act();
	rePhysics* physics() const;
	void physics(rePhysics* val);
	reInput* input() const;
	void input(reInput* val);

	reRenderer* renderer() const;
	void renderer(reRenderer* val);
};

#endif // engine_h__
