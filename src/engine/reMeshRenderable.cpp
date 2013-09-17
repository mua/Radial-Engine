#include "reMeshRenderable.h"
#include "reRadial.h"
#include "reAssetLoader.h"
#include "reMesh.h"

reMeshRenderable::reMeshRenderable()
{
	_mesh = 0;
	bonesMatched = false;
}

reMeshRenderable::~reMeshRenderable()
{
	reRadial::shared()->assetLoader()->releaseShared(mesh());
}

void reMeshRenderable::loadJson( reVar& value )
{
	reRenderable::loadJson(value);
	bonesMatched = false;
	std::string fname = value["meshFileName"].asString();
	meshFileName(fname);
}

void reMeshRenderable::toJson( reVar& val, int categories /*= ALL_PROPERTIES*/ )
{
	reRenderable::toJson(val);
	val["meshFileName"] = meshFileName();
}

std::string reMeshRenderable::meshFileName() const
{
	return _mesh ? reRadial::shared()->assetLoader()->relativePath(mesh()->path()) : "";
}

void reMeshRenderable::meshFileName( std::string val )
{
	_meshFileName = val;
	if (val.size())
	{
		mesh(reRadial::shared()->assetLoader()->loadShared<reMesh>(val));
	}	
}

reMesh* reMeshRenderable::mesh() const
{
	return _mesh;
}

void reMeshRenderable::mesh( reMesh* val )
{
	_mesh = val;
	boneLinks = _mesh->boneLinks;
	bBox(val->bBox());
}

void reMeshRenderable::updateShader( reShader* shader )
{
	if (mesh()->hasSkin)
	{
		if (!bonesMatched) matchBones();
		reMat4 bones[32];
		for ( size_t i=0; i<boneLinks.size(); i++)
		{
			if (boneLinks[i].bone)
			{
				reMat4 boneTransform(boneLinks[i].bone->worldTransform().matrix);
				reMat4 linkTransform(boneLinks[i].linkMatrix);
				bones[i] = boneTransform * linkTransform;
			}
		}
		shader->setUniformMatrix(reShader::boneMatrices, 32, glm::value_ptr(bones[0]));
	}
}

int reMeshRenderable::getEffect()
{
	return mesh()->hasSkin ? reEFFECT_SKIN : reEFFECT_DEFAULT;
}

void reMeshRenderable::matchBones()
{
	bonesMatched = true;
	for ( size_t i=0; i<boneLinks.size(); i++)
	{
		boneLinks[i].bone = (reNode*)(node->parent()->parent())->objectByName(string(&boneLinks[i].name[0]), true);
		assert(boneLinks[i].bone);
	}
}

void reMeshRenderable::draw( int state /*= 0 */, int id /*= 0 */ )
{

}

std::string reMeshRenderable::className()
{
	return "reMeshRenderable";
}

void reMeshRenderable::getTasks( reRenderTask& parent, reTaskList& tasks )
{
	reRenderTask task(parent);
	task.clients.push_back(this);
	for (size_t i=0; i<mesh()->groups.size(); i++)
	{
		mesh()->groups[i]->getTasks(task, tasks);
	}
}

