#include "reNode.h"

#include "reMeshRenderable.h"
#include "reMesh.h"
#include "reRadial.h"
#include "reAssetLoader.h"
#include "reShader.h"

#include <fstream>
#include <algorithm>

#include "rePhysics.h"

reNode::reNode()
{
	id = reRadial::shared()->registerNode(this);
	add(children = new reNodeCollection, true);
	children->name("children");
	add(renderables = new reRenderableCollection, true);
	renderables->name("renderables");
	_parent = 0;
}

reNode::~reNode()
{
	dispatchMessage (&reMessage(reM_NODE_DESTRUCT));
	reRadial::shared()->unregisterNode(id);

	if (parent())
	{
		parent()->children->remove(this);
	}
}

void reNode::save( string& fileName )
{
	ofstream fs(fileName.c_str(), ios::out);
	if (!fs.fail())
	{
		Json::StyledWriter writer;
		reVar val;
		toJson(val);
		fs << writer.write(val.asJson());
		fs.close();		
	}
}

void reNode::load( string& fileName )
{
	ifstream fs(fileName.c_str(), ios::in);
	if (!fs.fail())
	{
		Json::Reader reader;		
		Json::Value root;
		reader.parse(fs, root);
		loadJson(reVar(root));
		fs.close();
	}
}

void reNode::toJson(reVar& val, int categories)
{
	reCollection::toJson(val, categories);
	reVar transformValue;
	for ( size_t i=0; i<16; i++)
	{
		transformValue[i] = transform().matrix[i/4][i%4];
	}
	val["transform"] = transformValue;	
}

void reNode::loadJson( reVar& val )
{
	if (val.isMember("renderables"))
	{
		renderables->loadJson(val["renderables"]);
	}
	reCollection::loadJson(val);

	// context specific stuff
	reVar& transformValues = val["transform"];
	reTransform transform;
	for (unsigned int i=0; i<transformValues.size(); i++)
	{
		transform.matrix[i/4][i%4] = transformValues[i].asFloat();
	}
	this->transform(transform);
	for ( size_t i=0; i<count(); i++)
	{
		reNode* child = dynamic_cast<reNode*>(at(i));
		if (child)
		{
			remove(child);
			children->add(child);
		}
	}
	afterLoad();
}

reNode* reNode::parent() const
{
	return _parent;
}

void reNode::parent( reNode* val )
{
	_parent = val;
	updateWorldTransform();
}

void reNode::render(int state)
{
	glPushMatrix();
	glMultMatrixf(glm::value_ptr(worldTransform().matrix));

	if (state == reRS_SELECT)
	{
		glColor4ub(id >> 16, id >> 8, id & 255, 255);
	}
	for ( size_t i=0; i<renderables->count(); i++)
	{
		reRenderable* renderable = (reRenderable*)renderables->at(i);
		renderable->draw(state, id);
	}
	glPopMatrix();
	for ( size_t i=0; i<children->count(); i++)
	{
		reNode* node = (reNode*)children->at(i);
		node->render(state);
	}
}

std::string reNode::className()
{
	return "reNode";
}

void reNode::processMessage( reMessage* message )
{
	if (parent())
	{
		parent()->processMessage(message);
	}
	dispatchMessage(message);
}

reBBox reNode::bBox() const
{
	return _bBox;
}

void reNode::bBox( const reBBox val )
{
	_bBox = val;
}

void reNode::updateBBox()
{
	reBBox bbox;
	for (size_t i=0; i<children->count(); i++)
	{
		children->at(i)->updateBBox();
		bbox.addBox(children->at(i)->bBox(), children->at(i)->transform().matrix);
	}
	for (size_t i=0; i<renderables->count(); i++)
	{
		bbox.addBox(renderables->at(i)->bBox());
	}
	bBox(bbox);
}

reTransform reNode::transform() const
{
	return _transform;
}

void reNode::transform( reTransform val )
{
	_transform = val;
	updateWorldTransform();
}

reTransform reNode::worldTransform() const
{
	return _worldTransform;
}

void reNode::worldTransform( reTransform val )
{
	transform(reTransform(objectTransform().inverseMatrix() * val.matrix));
}

reTransform reNode::objectTransform() const
{
	return parent() ? parent()->worldTransform() : reTransform();
}

void reNode::updateWorldTransform()
{
	_worldTransform.matrix = objectTransform().matrix * transform().matrix;
	for ( size_t i=0; i<children->count(); i++)
	{
		((reNode*)children->at(i))->updateWorldTransform();
	}
	dispatchMessage(&reMessage(reM_TRANSFORM));
}

bool reNode::isChildNode(reNode* child)
{
	reNode *node = child;
	while (node = node->parent()) 
	{
		if (node == this)
			return true;
	}
	return false;
}

reWalkState reNode::walk(reWalkState (*callback)(reNode*, void*), void* ctx)
{
	reWalkState state, childState;
	if (state = callback(this, ctx))
		return state;

	for ( size_t i=0; i<children->count(); i++)
	{
		reNode* node = (reNode*)children->at(i);
		childState = node->walk(callback, ctx);
		if (childState == reWS_COMPLETE)
		{
			return reWS_COMPLETE;
		}
	}
	return reWS_GO;
}

//////////////////////////////////////////////////////////////////////////

struct _FilterCtx
{
	reWalkState (*callback)(reNode*, void*);
	void* ctx;
	reNodeVector* out;
};
reWalkState filterCalback(reNode* node, void* ctx)
{
	_FilterCtx *mctx = (_FilterCtx*)ctx;
	reWalkState ret = mctx->callback(node, mctx->ctx);
	if (ret & reWS_ADD)
		mctx->out->push_back(node);
	return ret;
}
void reNode::filter(reWalkState (*callback)(reNode*, void*), void* ctx, reNodeVector& out)
{
	_FilterCtx mctx;
	mctx.ctx = ctx;
	mctx.callback = callback;
	mctx.out = &out;
	walk(filterCalback, &mctx);
}

reStringList reNode::acceptedClasses()
{
	reStringList ret;
	ret.push_back("reBody");
	return ret;
}

void reNode::afterLoad()
{

}

void reNode::getTasks( reRenderTask& parent, reTaskList& tasks )
{
	for (size_t i=0; i<renderables->count(); i++)
	{
		reRenderTask task(parent);
		task.transform.matrix = task.transform.matrix * transform().matrix;
		//task.clients.push_back(this);
		renderables->at(i)->getTasks(task, tasks);
	}
	for (size_t i=0; i<children->count(); i++)
	{
		reRenderTask task(parent);
		//task.clients.push_back(this);
		task.transform.matrix = task.transform.matrix * transform().matrix;
		children->at(i)->getTasks(task, tasks);
	}
}

void reNode::updateShader( reShader* shader )
{

}

//////////////////////////////////////////////////////////////////////////

void reNodeCollection::insert(reObject* object, int index)
{
	reCollection::insert(object, index);
	((reNode*)object)->parent((reNode*)this->super);
}

std::string reNodeCollection::className()
{
	return "reNodeCollection";
}

reStringList reNodeCollection::acceptedClasses()
{
	reStringList ret;
	ret.push_back("reNode");
	ret.push_back("reScatter");
	return ret;
}

//////////////////////////////////////////////////////////////////////////

std::string reRenderableCollection::className()
{
	return "reRenderableCollection";
}

void reRenderableCollection::toJson(reVar& val, int categories)
{
	reCollection::toJson(val, categories);
}

void reRenderableCollection::loadJson( reVar& value )
{
	/*
	if (value.kind == reVar::seARRAY)
	{
		for (unsigned int i=0; i<value.size(); i++)
		{
			reMesh *mesh = new reMesh;
			mesh->load(value[i].asString());
			add(mesh);
		}
		return;
	}
	*/
	reCollection::loadJson(value);
}

void reRenderableCollection::add(reObject* object, bool fixed)
{
	reCollection::add(object);
	if (reMeshRenderable* mesh = dynamic_cast<reMeshRenderable*>(object))
	{
		reBBox bBox = ((reNode*)super)->bBox();
		bBox.addBox(mesh->mesh()->bBox());
		((reNode*)super)->bBox(bBox);
		mesh->node = (reNode*)super;
		reMessageRenderable msg;
		msg.id = reM_NODE_ADD_RENDERABLE;
		msg.renderable = mesh;
		mesh->node->dispatchMessage(&msg);
	}
}

reStringList reRenderableCollection::acceptedClasses()
{
	reStringList ret;
	ret.push_back("reRenderable");
	ret.push_back("reMesh");
	ret.push_back("reAnimator");
	return ret;
}