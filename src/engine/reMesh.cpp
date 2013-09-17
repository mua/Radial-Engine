#include "reMesh.h"

#include "reRadial.h"
#include "reAssetLoader.h"
#include "core.h"
#include "reNode.h"
#include "reBone.h"

#include <windows.h>
#include <core.h>

#include <fstream>

#include "reVertexBuffer.h"
#include "reCamera.h"

reMesh::reMesh()
{
	loaded = false;
	hasSkin = false;
}

reMesh::~reMesh()
{
	for (reGroupVector::iterator it = groups.begin(); it != groups.end(); it++)
	{
		delete *it;
	}
}

void reMesh::addFace( reFace& face, int matId )
{
	reMaterial* mat = materialSet.materialById(matId);
	reGroup* group = groupByMaterial(mat);
	if (!group)
	{
		group = new reGroup;
		group->material = mat;
		groups.push_back(group);
	}
	group->faces.push_back(face);
}

reGroup* reMesh::groupByMaterial( reMaterial* material )
{
	for (unsigned int i=0; i<groups.size(); i++)
	{
		if (*groups[i]->material == *material)
		{
			return groups[i];
		}
	}
	return NULL;
}

void reMesh::save( std::string& fileName )
{
	path(fileName);
	std::ofstream fs(fileName.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	save(fs);
	fs.close();
}

void reMesh::save( std::ostream& os )
{
	os.write((char*)&hasSkin, sizeof(hasSkin));
	if (hasSkin)
	{
		int count = boneLinks.size();
		os.write((char*)&(count), sizeof(count));
		os.write((char*)&boneLinks[0], sizeof(reBoneLink)*count);
	}
	materialSet.save(os);
	unsigned int count = groups.size();
	os.write((char*)&count, sizeof(count));
	for (reGroupVector::iterator it = groups.begin(); it!=groups.end(); it++)
	{
		(*it)->save(os);
	}
}

void reMesh::load( std::string& fileName )
{	
	if (fileName == path() && loaded)
	{
		return;
	}
	std::ifstream fs(reRadial::shared()->assetLoader()->filePath(fileName).c_str(), std::ios::in | std::ios::binary);	
	if (!fs.fail())
	{		
		path(fileName);
		load(fs);
		fs.close();	
	}
}

void reMesh::load( std::istream& is )
{
	loaded = true;
	is.read((char*)&hasSkin, sizeof(hasSkin));
	if (hasSkin)
	{
		int count;
		is.read((char*)&(count), sizeof(count));
		boneLinks.resize(count);
		is.read((char*)&boneLinks[0], sizeof(reBoneLink)*count);
		shader.vsFileName("shaders/skin.vp");
		shader.fsFileName("shaders/skin.fp");
	}
	materialSet.load(is);
	unsigned int count;
	is.read((char*)&count, sizeof(count));
	for (unsigned int i=0; i<count; i++)
	{
		reGroup* group = new reGroup;
		group->load(is, materialSet);
		groups.push_back(group);
	}
	_bBox = reBBox();
	for (reGroupVector::const_iterator git=groups.begin(); git!=groups.end(); git++)
	{
		for (reFaceVector::const_iterator fit=(*git)->faces.begin(); fit != (*git)->faces.end(); fit++)
		{
			for (unsigned int i=0; i<3; i++)
			{
				_bBox.addPoint((*fit).vertices[i].pos);
			}
		}
	}
}

void reMesh::load()
{
	load(path());
}

std::string reMesh::path() const
{
	return _path;
}

void reMesh::path( std::string val )
{
	_path = val;
}

reBBox reMesh::bBox() 
{
	return _bBox;
}

void reMesh::bBox( reBBox val )
{
	_bBox = val;
}

std::string reMesh::className()
{
	return "reMesh";
}

void reMesh::loadFromFile( string fileName )
{
	load(fileName);
}

//////////////////////////////////////////////////////////////////////////

void reGroup::save( std::ostream& os )
{
	int matID = material ? material->id : -1;
	os.write((char*)&matID, sizeof(matID));
	unsigned int count = faces.size();
	os.write((char*)&count, sizeof(count));
	for (reFaceVector::iterator it=faces.begin(); it != faces.end(); it++)
	{
		(*it).save(os);
	}
}

void reGroup::load( std::istream& is, reMaterialSet& materials )
{
	int matID;
	reBBox abBox;
	is.read((char*)&matID, sizeof(matID));
	material = materials.materialById(matID);
	isTransparent = material && material->isTransparent;
	unsigned int count;
	is.read((char*)&count, sizeof(count));
	for (unsigned int i=0; i<count; i++)
	{
		reFace face;
		face.load(is);
		faces.push_back(face);
		for (unsigned int i=0; i<3; i++)
		{
			abBox.addPoint(face.vertices[i].pos);
		}
	}	
	bufferChanged = true;
	bBox(abBox);
}

reGroup::~reGroup()
{
	delete vertexBuffer;
}

reGroup::reGroup()
{
	vertexBuffer = new reVertexBuffer();
	material = 0;
	bufferChanged = false;
}

void reGroup::updateBuffer()
{
	vertexBuffer->load(&faces[0].vertices[0], faces.size()*3);
	bufferChanged = false;
}

void reGroup::draw( int state /*= 0*/, int id /*= 0*/ )
{
	//std::cout << faces.size() << std::endl;
	if (isTransparent)
	{
		//glDepthMask(0);
	}
	glDrawArrays(GL_TRIANGLES, 0, faces.size()*3);
	if (isTransparent)
	{
		//glDepthMask(1);
	}
}

void reGroup::updateShader( reShader* shader )
{
	if (bufferChanged)
	{
		updateBuffer();
	}	
	vertexBuffer->bindPointers(shader);
}

void reGroup::getTasks( reRenderTask& parent, reTaskList& tasks )
{
	reRenderTask task(parent);
	task.clients.push_back(this);
	task.clients.push_back(material);
	task.renderable = this;
	tasks.push_back(task);
}

//////////////////////////////////////////////////////////////////////////

void reFace::save( std::ostream& os )
{
	os.write((char*)&(vertices), sizeof(vertices));
}

void reFace::load( std::istream& is)
{
	vertices[0].weights = vertices[1].weights = vertices[2].weights = reVec4(0,0,0,0);
	vertices[0].bones = vertices[1].bones = vertices[2].bones = reVec4(-1,-1,-1,-1);
	is.read((char*)&vertices, sizeof(vertices));
}