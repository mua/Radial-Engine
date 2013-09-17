#include "rePTerrain.h"
#include "reShader.h"
#include "reVertexBuffer.h"
#include "reRadial.h"
#include "reCamera.h"
#include "reAssetLoader.h"
#include "reNoise.h"
#include "reBody.h"
#include "reCamera.h"
#include "reMesh.h"

#include <algorithm>
#include "reBody.h"

rePTerrain::rePTerrain()
{
	camera = 0;
	player = 0;
	collisionRows = 9; collisionCols = 9;
	collisionHeight = 1; collisionWidth = 1;
	heightmap = new float[(collisionRows+1)*(collisionCols+1)];
	ZeroMemory(heightmap, sizeof(float) * collisionRows*collisionCols);
	terrainRenderable = new rePTerrainRenderable;
	terrainRenderable->node = this;	
	terrainRenderable->updateBBox();
	add(body = new reBody, true);
	body->kind(reBody::reKinematic);
	body->mass(0);
	body->addRenderable(terrainRenderable);
	observe(reRadial::shared(), reM_TIMER);

//	body->btBody->setCcdSweptSphereRadius(0.3f);
}

std::string rePTerrain::className()
{
	return "rePTerrain";
}

void rePTerrain::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	if (player)
	{
		reVec3 pos = player->worldTransform().position();
		pos.y = 0;
		pos.x = int(pos.x/terrainRenderable->size().x)*terrainRenderable->size().x;
		pos.z = int(pos.z/terrainRenderable->size().y)*terrainRenderable->size().y;
		transform(glm::translate(reMat4(), pos));
		updateHeightmap();
	}
}

void rePTerrain::updateHeightmap()
{
	reVec3 center = worldTransform().position();
	center.y = 0;
	body->readTransform();
	reVec3 offset3 = center - reVec3(collisionCols*collisionWidth/2, 0, collisionRows*collisionHeight/2);
	reVec4 offset(offset3.x, offset3.y, offset3.z, 0);
	int i=0;
	for ( size_t y=0; y<collisionRows+1; y++)
	{
		for ( size_t x = 0; x<collisionCols+1; x++)		
		{
			reVec4 pos = reVec4(x*collisionWidth, 0, y*collisionHeight, 0) + offset;
			heightmap[i] = calculateVertex(pos).y;
			i+=1;
		}
	}
}

void rePTerrain::afterLoad()
{
	renderables->add(terrainRenderable, true);
}


void drawQuad2(reVec3 a, reVec3 b) 
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(a[0], a[1], a[2]);
	glVertex3f(a[0], a[1], b[2]);
	glVertex3f(a[0], b[1], b[2]);
	glVertex3f(b[0], b[1], b[2]);
	glVertex3f(b[0], b[1], a[2]);
	glVertex3f(b[0], a[1], a[2]);
	glVertex3f(a[0], a[1], a[2]);
	glEnd();
}

void drawTileBox(reVec3 a, reVec3 b)
{
	glDisable(GL_CULL_FACE);	
	drawQuad2(reVec3(a[0], a[1], a[2]), reVec3(a[0], b[1], b[2]));
	drawQuad2(reVec3(a[0], a[1], a[2]), reVec3(b[0], a[1], b[2]));
	drawQuad2(reVec3(a[0], a[1], a[2]), reVec3(b[0], a[1], a[2]));
	drawQuad2(reVec3(b[0], b[1], b[2]), reVec3(b[0], a[1], a[2]));
	drawQuad2(reVec3(b[0], b[1], b[2]), reVec3(a[0], b[1], a[2]));
	drawQuad2(reVec3(b[0], b[1], b[2]), reVec3(a[0], b[1], b[2]));	
	glEnable(GL_CULL_FACE);
}

void getTiles (vector<reBBox>& boxes, reVec3 a, reVec3 b, reFrustum& frustum, int depth = 3)
{
	reVec3 center = (a+b)*0.5f;
	reVec3 points[] = {a, b};
	reVec3 box[2];
	depth--;
	for ( size_t i=0; i<8; i++)
	{
		box[0] = box[1] = center;
		int x = (i&1), y = (i&2)>>1, z = (i&4)>>2;
		box[x].x = points[x].x; box[y].y = points[y].y; box[z].z = points[z].z;
		reBBox bbox(box[0], box[1]);

		int test = frustum.classifyBBox(bbox);
		if(test == 1 || (test && !depth))
		{
			boxes.push_back(bbox);
		}
		if (depth > 0 && test)
		{
			getTiles(boxes, box[0], box[1], frustum, depth);
		}	
	}
}

//////////////////////////////////////////////////////////////////////////

rePTerrainRenderable::rePTerrainRenderable()
{
	node = 0;
	columns(256);
	rows(256);
	size(reVec2(2, 2));
	textures[3] = new reTexture();
	textures[3]->fileName("/materials/textures/tiles/arctic/Snow0095_2_S.jpg");
	textures[1] = new reTexture();
	textures[1]->fileName("materials/textures/tiles/arctic/Snow0041_5_S.jpg");
	textures[2] = new reTexture();
	textures[2]->fileName("materials/textures/tiles/arctic/Snow0041_5_S.jpg");
	textures[0] = new reTexture();
	textures[0]->fileName("materials/textures/tiles/arctic/Snow0041_5_S.jpg");
	mesh = new reMesh;
	load();
}

void rePTerrainRenderable::load()
{
	reGroup* group = new reGroup;

	reVec3 offset(-columns()*size().x/2, 0, -rows()*size().y/2);
	for ( size_t x=0; x<columns(); x++)
		for ( size_t y=0; y<rows(); y++)
		{
			float x1 = x*size().x;
			float y1 = y*size().y;
			float x2 = (x+1)*size().x;
			float y2 = (y+1)*size().y;
			reFace face1, face2;
			face1.vertices[0] = reVertex(reVec3(x1, 0, y2)+ offset);
			face1.vertices[1] = reVertex(reVec3(x2, 0, y1)+ offset);
			face1.vertices[2] = reVertex(reVec3(x1, 0, y1)+ offset);			
			face2.vertices[0] = reVertex(reVec3(x1, 0, y2)+ offset);
			face2.vertices[1] = reVertex(reVec3(x2, 0, y2)+ offset);
			face2.vertices[2] = reVertex(reVec3(x2, 0, y1)+ offset);					
			group->faces.push_back(face1);
			group->faces.push_back(face2);
		}
	mesh->groups.push_back(group);
	group->bufferChanged = true;
}

int rePTerrainRenderable::columns() const
{
	return _columns;
}

void rePTerrainRenderable::columns( int val )
{
	_columns = val;
	updateBBox();
}

int rePTerrainRenderable::rows() const
{
	return _rows;
}

void rePTerrainRenderable::rows( int val )
{
	_rows = val;
	updateBBox();
}

reVec2 rePTerrainRenderable::size() const
{
	return _size;
}

void rePTerrainRenderable::size( reVec2 val )
{
	_size = val;
	updateBBox();
}

void rePTerrainRenderable::getHeigtmap(int x0, int y0, int x1, int y1, float vertices[])
{
	int i=0;
	reVec3 offset(-int(columns()/2)*size().x, 0, -int(rows()/2)*size().y);
	for ( size_t y=y0; y<y1; y++)
		for ( size_t x=x0; x<x1; x++)		
		{
			float x1 = x*size().x;
			float y1 = y*size().y;
			reVec4 pos = reVec4(x1, 0, y1, 1);
			pos.x+=offset.x; pos.z +=offset.z;
			reVec4 wpos = node->worldTransform().matrix * pos;
			vertices[i++] = calculateVertex(wpos).y;
		}
};

reVec3 rayPlaneIntersection(reRay a, reRay plane)
{
	float t = glm::dot(plane.n, a.p - plane.p) / -glm::dot(plane.n, a.n);
	return t * a.n + a.p;
}

reRay planePlaneIntersection(reRay a, reRay b)
{
	reVec3 normal = glm::normalize(glm::cross(a.n, b.n)); 
	reVec3 dir = glm::normalize(glm::cross(normal, a.n));
	return reRay(rayPlaneIntersection(reRay(a.p, dir), b), normal);
};

void rePTerrainRenderable::draw( int state /*= 0*/, int id /*= 0*/ )
{	
	reFrustum frustum;
	rePTerrain* terrain = (rePTerrain*)node;	
	if (terrain->camera)
	{
		frustum.fromMatrix(terrain->camera->projection * terrain->camera->view);
		/*
		reRay topLeft = planePlaneIntersection(frustum.top, frustum.left);
		reRay topRight = planePlaneIntersection(frustum.top, frustum.right);
		reRay bottomLeft = planePlaneIntersection(frustum.bottom, frustum.left);
		reRay bottomRight = planePlaneIntersection(frustum.bottom, frustum.right);

		reVec3 tlb = rayPlaneIntersection(topLeft, frustum.start);
		reVec3 trb = rayPlaneIntersection(topRight, frustum.start);
		reVec3 blb = rayPlaneIntersection(bottomLeft, frustum.start);
		reVec3 brb = rayPlaneIntersection(bottomRight, frustum.start);

		reVec3 tlf = rayPlaneIntersection(topLeft, frustum.end);
		reVec3 trf = rayPlaneIntersection(topRight, frustum.end);
		reVec3 blf = rayPlaneIntersection(bottomLeft, frustum.end);
		reVec3 brf = rayPlaneIntersection(bottomRight, frustum.end);

		glPushMatrix();
		reRadial::shared()->camera->apply();
		glColor4f(1,0,1,1);
		glLineWidth(2);
		glBegin(GL_LINES);

		reVec3 s = reVec3(terrain->camera->transform.matrix * reVec4(0,0,0,1));
		
		glColor4f(1,1,0,1);
		glVertex3fv(glm::value_ptr(frustum.top.p));		
		glColor4f(1,0,0,1);
		glVertex3fv(glm::value_ptr(frustum.top.n + frustum.top.p));
		glColor4f(1,1,0,1);
		glVertex3fv(glm::value_ptr(frustum.bottom.p));
		glColor4f(1,0,0,1);
		glVertex3fv(glm::value_ptr(frustum.bottom.n + frustum.bottom.p));
		glColor4f(1,1,0,1);
		glVertex3fv(glm::value_ptr(frustum.left.p));
		glColor4f(1,0,0,1);
		glVertex3fv(glm::value_ptr(frustum.left.n + frustum.left.p));
		glColor4f(1,1,0,1);
		glVertex3fv(glm::value_ptr(frustum.right.p));
		glColor4f(1,0,0,1);
		glVertex3fv(glm::value_ptr(frustum.right.n + frustum.right.p));
		glColor4f(1,1,0,1);
		glVertex3fv(glm::value_ptr(frustum.end.p));
		glColor4f(1,0,0,1);
		glVertex3fv(glm::value_ptr(frustum.end.n + frustum.end.p));
		glColor4f(1,1,0,1);
		glVertex3fv(glm::value_ptr(frustum.start.p));
		glColor4f(1,0,0,1);
		glVertex3fv(glm::value_ptr(frustum.start.n + frustum.start.p));
		*/
		/*
		glColor4f(0,0,1,1);
		glVertex3fv(glm::value_ptr(tlb));
		glVertex3fv(glm::value_ptr(trb));
		glVertex3fv(glm::value_ptr(trb));
		glVertex3fv(glm::value_ptr(brb));
		glVertex3fv(glm::value_ptr(brb));
		glVertex3fv(glm::value_ptr(blb));
		glVertex3fv(glm::value_ptr(blb));
		glVertex3fv(glm::value_ptr(tlb));
		
		glVertex3fv(glm::value_ptr(tlf));
		glVertex3fv(glm::value_ptr(trf));
		glVertex3fv(glm::value_ptr(trf));
		glVertex3fv(glm::value_ptr(brf));
		glVertex3fv(glm::value_ptr(brf));
		glVertex3fv(glm::value_ptr(blf));
		glVertex3fv(glm::value_ptr(blf));
		glVertex3fv(glm::value_ptr(tlf));
		/*
		glVertex3fv(glm::value_ptr(brb));
		glVertex3fv(glm::value_ptr(tlf));
		glVertex3fv(glm::value_ptr(trb));		
		glVertex3fv(glm::value_ptr(trf));

		glVertex3fv(glm::value_ptr(blb));
		glVertex3fv(glm::value_ptr(blf));
		glVertex3fv(glm::value_ptr(brb));		
		glVertex3fv(glm::value_ptr(brf));
		*/
		/*
		glEnd();
		glLineWidth(1);
		glPopMatrix();
		*/
	}
	
	if (!state)
	{
		//drawTiles(frustum);
		/*
		shader.use();
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glDisable(GL_CULL_FACE);
		groups[0]->vertexBuffer->bindPointers();
		shader.setUniformMatrix("viewMatrix", 1, glm::value_ptr(reRadial::shared()->camera->view));
		reMat4 mat(node->worldTransform().matrix);
		shader.setUniformMatrix("modelMatrix", 1, glm::value_ptr(mat));
		shader.setUniform("cellSize", 3.0f*size());
		int buffer[4];
		for ( size_t i=0; i<4; i++)
		{
			buffer[i] = i;
			glActiveTexture(GL_TEXTURE0 + i);
			glEnable(GL_TEXTURE_2D);
			textures[i]->bind(0);
		}
		shader.setUniform("splatTiles[0]", 4, buffer);
		//glDrawArrays(GL_TRIANGLES, 0, groups[0]->faces.size()*3);
		*/
		drawTiles(frustum);
		/*
		for ( size_t i=0; i<4; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glDisable(GL_TEXTURE_2D);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		shader.unuse();
		*/
	}
}

void rePTerrainRenderable::drawTiles( reFrustum &frustum )
{
	return;
	/*
	reMat4 mat(node->worldTransform().matrix);	
	reVec3 tileSize(size().x*columns(), size().x*columns()/3, size().y*rows());
	frustum.transform(glm::inverse(mat));

	int multiple = 0;
	vector<reBBox> boxes;
	float distance = glm::dot(frustum.end.n, frustum.start.p - frustum.end.p);
	getTiles(boxes, reVec3(distance, 100*distance, distance), reVec3(-distance, -100*distance, -distance), frustum, 3);
	vector<reVec3> list;
	list.reserve(1000);
	for ( size_t i=0; i<boxes.size(); i++)
	{
		reVec3 tileBegin = boxes[i].pMin / tileSize;
		tileBegin.x = int(tileBegin.x); tileBegin.z = int(tileBegin.z); tileBegin.y = int(tileBegin.y);		
		reVec3 tileEnd = boxes[i].pMax / tileSize;
		tileEnd.x = int(tileEnd.x); tileEnd.z = int(tileEnd.z); tileEnd.y = int(tileEnd.y);
		int y = 0;
		for ( int x = tileBegin.x; x<=tileEnd.x; x+=1)
			//for ( size_t y = tileBegin.y; y<=tileEnd.y; y+=1)
			for ( int z = tileBegin.z; z<=tileEnd.z; z+=1)
			{
				glColor4f(1,0,0,.5);
				//drawTileBox(reVec3(x*tileSize.x, y*tileSize.y, z*tileSize.z), reVec3((x+1)*tileSize.x, (y+1)*tileSize.y, (z+1)*tileSize.z));
				reVec3 center = (reVec3(x*tileSize.x, y*tileSize.y, z*tileSize.z) + reVec3((x+1)*tileSize.x, (y+1)*tileSize.y, (z+1)*tileSize.z))/2.0f;
				
				center.y = 0;
				reMat4 tileTransform = mat * glm::translate(reMat4(), center);
				//shader.setUniformMatrix("worldMatrix", 1, glm::value_ptr(tileTransform));				

				if (find(list.begin(), list.end(), center) == list.end())
				{
					glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
					glDrawArrays(GL_TRIANGLES, 0, groups[0]->faces.size()*3);
					list.push_back(center);
				}
				else
					multiple++;
			}
			/*
			glLineWidth(2);
			glColor4f(1,0,0,.5);
			//drawTileBox(boxes[i].pMax, boxes[i].pMin);
			glLineWidth(1);	
			
	}
*/
}

int rePTerrainRenderable::getEffect()
{
	return reEFFECT_PROCTERRAIN | reEFFECT_DIFFUSE_TEXTURE;
}

void rePTerrainRenderable::updateShader( reShader* shader )
{
	shader->setUniform(reShader::cellSize, 3.0f*size());
	shader->setTexture(reShader::splatTextures, textures[0], 0);
	shader->setTexture(reShader::splatTextures, textures[1], 1);
	shader->setTexture(reShader::splatTextures, textures[2], 2);
	shader->setTexture(reShader::splatTextures, textures[3], 3);
}

void rePTerrainRenderable::getTasks( reRenderTask& parent, reTaskList& taskList )
{
	reRenderTask task(parent);
	task.clients.push_back(this);
	mesh->groups[0]->getTasks(task, taskList);
}

void rePTerrainRenderable::updateBBox()
{
	reVec2 diag(columns()*size().x, rows()*size().y);
	diag /= 2;
	if (node)
	{
		node->bBox(reBBox(reVec3(-diag.x, 0, -diag.y), reVec3(diag.x, 0, diag.y)));
	}	
}

