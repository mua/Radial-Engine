#include "reRenderer.h"
#include "reCamera.h"
#include "reNode.h"
#include "reShader.h"
#include "reLight.h"

reRenderer::reRenderer()
{
	initialized = false;
	passes.push_back(new reShadowPass(this, 0));
	passes.push_back(new reRenderPass(this, 1));
	defaultLight = new reLight;
}

reRenderer::~reRenderer()
{

}

void reRenderer::push( reRenderTask& parentTask, reRenderClient* client )
{/*
	assert(client);
	parentTask.clients.push_back(client);
	client->updateTask(parentTask);
	reRenderClientList subClients;
	client->subClients(subClients);
	if (subClients.size())
	{
		for (size_t i=0; i<subClients.size(); i++)
		{
			reRenderTask task(parentTask);
			task.clients.reserve(10);
			push(task, subClients[i]);
		}
	}
	else
	{
		reRenderTask task = parentTask;
		task.updateEffects();
		tasks.push_back(task);
	}	*/
}

reWalkState reRenderer_walker(reNode* node, void* ctx)
{
	reRenderer* renderer = (reRenderer*)ctx;
	reRenderTask task;
	renderer->push(task, node);
	return reWS_GO;
}

reMat4 camProjView;

bool sortTasks(const reRenderTask& t1, const reRenderTask& t2)
{
	if (t1.renderable->isTransparent != t2.renderable->isTransparent)
	{
		return t1.renderable->isTransparent ? 0 : 1;
	}
	reVec4 c1 = t1.transform.matrix * reVec4(t1.renderable->bBox().center(), 1.0f);
	reVec4 c2 = t1.transform.matrix * reVec4(t1.renderable->bBox().center(), 1.0f);
	return t1.renderable->isTransparent ? ((camProjView * c1).z < (camProjView * c2).z) : ((camProjView * c1).z > (camProjView * c2).z);
};

void reRenderer::render( reCamera* camera, reNode* node )
{
	if (!initialized)
	{
		initialize();
	}
	this->camera = camera;	
	tasks.clear();	
	tasks.reserve(200);
	//node->walk(reRenderer_walker, this);	

	tasks.clear();
	reRenderTask task;
	node->getTasks(task, tasks);
	for (int i=0; i<tasks.size(); i++)
	{
		tasks[i].updateEffects();
	}

	camProjView = this->camera->projection * this->camera->view;
	std::sort(tasks.begin(), tasks.end(), sortTasks);

	lights.clear();
	node->findObjects(lights, false);
	if(lights.size() == 0)
	{
		lights.push_back(defaultLight);
	}
	for (size_t i=0; i<passes.size(); i++ )
	{
		passes[i]->render();
	}
}

reShader* reRenderer::getShader( int name )
{
	if (shaders.find(name) == shaders.end())
	{
		reShader* shader = new reShader();
		shader->fsFileName("shaders/main.fp");
		shader->vsFileName("shaders/main.vp");
		shader->effects = name;
		if (reEFFECT_PROCTERRAIN & name)
		{
			shader->defines.push_back("_PROC_TERRAIN_");
		}
		if (reEFFECT_SHADOW_CAST & name)
		{
			shader->defines.push_back("_SHADOW_CAST_");
		}
		if (reEFFECT_SHADOW_MAP & name)
		{
			shader->defines.push_back("_SHADOW_MAP_");
		}
		if (reEFFECT_DIFFUSE_TEXTURE & name)
		{
			shader->defines.push_back("_DIFFUSE_TEXTURE_");
		}
		if (reEFFECT_SKIN & name)
		{
			shader->defines.push_back("_SKIN_");
		}
		shaders[name] = shader;
	}
	return shaders[name];
}

void reRenderer::initialize()
{
	initialized = true;
}

void reRenderer::shadowPass()
{

}

//////////////////////////////////////////////////////////////////////////

reRenderTask::reRenderTask()
{
	renderable = 0;
}

reRenderTask::reRenderTask( const reRenderTask& task )
{
	clients = task.clients;
	renderable = task.renderable;
	shaderName = task.shaderName;
	transform = task.transform;
}

void reRenderTask::updateEffects()
{
	shaderName = 0;
	for (size_t i=0; i<clients.size(); i++)
	{
		if(clients[i])
			shaderName |= clients[i]->getEffect();
	}
}

//////////////////////////////////////////////////////////////////////////

int reRenderClient::getEffect()
{
	return reEFFECT_DEFAULT | reEFFECT_SHADOW_CAST;
}

reRenderClient * reRenderClient::delegate() const
{
	return _delegate;
}

void reRenderClient::delegate( reRenderClient * val )
{
	_delegate = val;
}

void reRenderClient::getTasks( reRenderTask& parent, reTaskList& tasks )
{

}

//////////////////////////////////////////////////////////////////////////

reRenderPass::reRenderPass(reRenderer* renderer, int index)
{
	initialized = false;
	this->renderer = renderer;
	this->index = index;
}

reRenderPass::~reRenderPass()
{

}

void reRenderPass::initialize()
{
	initialized = true;
}

void reRenderPass::render()
{
	if (!initialized) initialize();
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);
	draw();
	glPopAttrib();
}

void reRenderPass::draw()
{
	setupViewport();
	reShader* shader = 0;

	for (size_t i=0; i<renderer->tasks.size(); i++)
	{
		reRenderTask &task = renderer->tasks[i];
		if (!task.renderable) continue;
		int shaderName = getShaderName(task);
		if (!shader || shader->effects != shaderName)
		{
			shader = renderer->getShader(shaderName);
			shader->use();
		}				
		for (size_t k=0; k<index; k++ )
		{
			renderer->passes[k]->attach(shader);
		}
		updateShader(shader);
		draw(task, shader);
	}
	if (shader)
	{
		shader->unuse();
	}
}

void reRenderPass::draw( reRenderTask &task, reShader* shader )
{
	for (size_t c = 0; c<task.clients.size(); c++)
	{
		if (task.clients[c]) // NULL GELMEMELI
		{
			task.clients[c]->updateShader(shader);
		}		
	}
	reMat4 mat = task.transform.matrix;
	shader->setUniformMatrix(reShader::worldMatrix, 1, glm::value_ptr(mat));
	task.renderable->draw();
}

void reRenderPass::attach( reShader* shader )
{

}

int reRenderPass::getShaderName( reRenderTask& task )
{
	return task.shaderName;
}

void reRenderPass::updateShader( reShader* shader )
{
	renderer->camera->pushMatrices(shader);
}

void reRenderPass::setupViewport()
{
	glViewport(
		renderer->camera->viewport().x, 
		renderer->camera->viewport().y, 
		renderer->camera->viewport().z, 
		renderer->camera->viewport().w);
}

//////////////////////////////////////////////////////////////////////////

reShadowPass::reShadowPass( reRenderer* renderer, int index ):
	reRenderPass(renderer, index)
{
	biasMatrix = reMat4(
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
		);
}

void reShadowPass::initialize()
{
	reRenderPass::initialize();
	size = reVec2(1024, 1024);
	glGenFramebuffers(1, &bufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, bufferName);	

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);	
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, 0);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
	

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void reShadowPass::draw()
{
	light()->aspect(1);
	glDisable(GL_SCISSOR_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, bufferName);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
	glDepthMask(GL_TRUE);
	glClearColor(1,1,0,1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	//glDisable (GL_BLEND);
	//glDisable (GL_TEXTURE_2D);
	glDisable (GL_LIGHTING);
	glDisable (GL_MULTISAMPLE);
	reRenderPass::draw();
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	lightMatrix = light()->projection * light()->view;
	lightViewMatrix = light()->view;
	lightProjMatrix = light()->projection;
	light()->viewport(light()->viewport());	
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void reShadowPass::draw( reRenderTask &task, reShader* shader )
{
	if (!(task.shaderName & reEFFECT_PROCTERRAIN))
	{
		reRenderPass::draw(task, shader);
	}	
}

void reShadowPass::attach( reShader* shader )
{
	biasMatrix = reMat4(
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
		);
	shader->setUniformMatrix(reShader::biasMatrix, 1, glm::value_ptr(biasMatrix));
	shader->setUniformMatrix(reShader::lightMatrix, 1, glm::value_ptr(lightMatrix));
	shader->setUniformMatrix(reShader::lightViewMatrix, 1, glm::value_ptr(lightViewMatrix));
	shader->setUniformMatrix(reShader::lightProjMatrix, 1, glm::value_ptr(lightProjMatrix));
	shader->setTexture(reShader::shadowMap, colorTexture);
	shader->setUniform(reShader::lightNearDistance, light()->nearPlane());
	shader->setUniform(reShader::lightFarDistance, light()->farPlane());
}

int reShadowPass::getShaderName( reRenderTask& task )
{
	return reEFFECT_SHADOW_MAP | (task.shaderName & reEFFECT_PROCTERRAIN) | (task.shaderName & reEFFECT_SKIN) | (task.renderable->isTransparent ? reEFFECT_DIFFUSE_TEXTURE : 0) ; 
}

void reShadowPass::updateShader( reShader* shader )
{
	light()->pushMatrices(shader);
	shader->setUniform(reShader::nearDistance, light()->nearPlane());
	shader->setUniform(reShader::farDistance, light()->farPlane());
}

void reShadowPass::setupViewport()
{
	glViewport(0, 0, size.x, size.y);
}

reLight* reShadowPass::light()
{
	return renderer->lights[0];
}


//////////////////////////////////////////////////////////////////////////


int rePCFShadowPass::getShaderName( reRenderTask& task )
{
	return reEFFECT_SHADOW_MAP_PCF; 
}

