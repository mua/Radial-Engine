#ifndef rePTerrain_h__
#define rePTerrain_h__

#include "core.h"
#include "reNode.h"
#include "reTypes.h"
#include "reMaterial.h"

class reCamera;
class reBody;
class reMesh;

class rePTerrainRenderable: public reRenderable
{
private:
	reVec2 _size;
	int _rows, _columns;
	reTexture* textures[4];	
	reMesh* mesh;
public:	
	rePTerrainRenderable();
	void load();
	virtual void draw(int state = 0, int id = 0);

	void drawTiles( reFrustum &frustum );

	int columns() const;
	void columns(int val);
	int rows() const;
	void rows(int val);
	reVec2 size() const;
	void size(reVec2 val);
	void getHeigtmap(int x0, int y0, int x1, int y1, float vertices[]);
	void updateHeightmap();
	virtual int getEffect();
	virtual void updateShader(reShader* shader) override;
	virtual void getTasks( reRenderTask& task, reTaskList& taskList );
	void updateBBox();
};

class rePTerrain: public reNode, public reMessageObserver
{
public:
	int collisionRows, collisionCols;
	float collisionWidth, collisionHeight;
	reBody* body;
	reCamera* camera;
	reNode* player;
	rePTerrainRenderable* terrainRenderable;
	float* heightmap;

	rePTerrain();
	virtual string className() override;
	void updateHeightmap();
	virtual void afterLoad();

	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) override;
};

#endif // rePTerrain_h__
