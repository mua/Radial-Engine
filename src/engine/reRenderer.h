#ifndef reRenderer_h__
#define reRenderer_h__

#include <vector>
#include <map>

#include "core.h"
#include "reTypes.h"
#include "reTransform.h"

class reNode;
class reRenderable;
class reShader;
class reCamera;
class reMaterial;
class reRenderClient;
class reRenderer;
class reLight;

typedef std::vector<reRenderClient*> reRenderClientList;

enum reEffectType
{
	reEFFECT_DEFAULT = 0,
	reEFFECT_PROCTERRAIN = 1,
	reEFFECT_SHADOW_CAST = 2,
	reEFFECT_SHADOW_MAP = 4,
	reEFFECT_DIFFUSE_TEXTURE = 8,
	reEFFECT_DIFFUSE_COLOR = 16,
	reEFFECT_SHADOW_MAP_PCF = 32,
	reEFFECT_SKIN = 64
};

struct reRenderTask
{
	int shaderName;

	reRenderClientList clients;
	reRenderable* renderable;
	reTransform transform;
	reRenderTask();
	reRenderTask(const reRenderTask& task);
	void updateEffects();
};

typedef std::vector<reRenderTask> reTaskList;

class reRenderClient
{
private:
	reRenderClient *_delegate;
	reRenderClient * delegate() const;
	void delegate(reRenderClient * val);
public:
	virtual void updateShader(reShader* shader) = 0;
	virtual void getTasks(reRenderTask& parent, reTaskList& tasks);
	virtual int getEffect();
};

class reRenderPass
{
public:
	reRenderer* renderer;
	int index;
	reRenderPass(reRenderer* renderer, int index);
	virtual ~reRenderPass();
	bool initialized;
	virtual void initialize();
	virtual void attach(reShader* shader);
	void render();
	virtual void draw();

	virtual void draw( reRenderTask &task, reShader* shader );

	virtual void setupViewport();
	virtual void updateShader( reShader* shader );
	virtual int getShaderName(reRenderTask& task);
};

class reShadowPass: public reRenderPass
{
public:
	reVec2 size;
	GLuint bufferName, colorTexture, depthTexture;
	reMat4 biasMatrix, lightMatrix, lightViewMatrix, lightProjMatrix;

	reShadowPass(reRenderer* renderer, int index);
	virtual void draw();
	virtual void initialize();
	virtual int getShaderName(reRenderTask& task);
	virtual void updateShader( reShader* shader );
	virtual void setupViewport();
	reLight* light();
	void attach(reShader* shader);
	virtual void draw( reRenderTask &task, reShader* shader );
};

class rePCFShadowPass: public reShadowPass
{
	virtual int getShaderName(reRenderTask& task);
};

typedef std::vector<reRenderPass*> rePassList;
typedef std::vector<reLight*> reLightList;

class reRenderer
{	
friend class reRenderPass;
friend class reShadowPass;

private:
	reLight* defaultLight;
	reTaskList tasks;
	reLightList lights;
	std::map<int, reShader*> shaders;
	reNode *nodeCursor;
	reCamera* camera;
	reMaterial *materialCursor;
	bool initialized;
	rePassList passes;
public:
	reRenderer();
	virtual ~reRenderer();
	void initialize();
	void push( reRenderTask& parentTask, reRenderClient* client );
	void render( reCamera* camera, reNode* node );

	void renderPass( reCamera* camera );

	void shadowPass();
	reShader* getShader( int name );
};

#endif // reRenderer_h__
