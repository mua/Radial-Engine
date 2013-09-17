#ifndef reTerrainNode_h__
#define reTerrainNode_h__

#include "reNode.h"
#include "reRenderable.h"
#include <QImage>
#include <QGLFrameBufferObject>
#include "reShader.h"

class QPainter;
class reTerrainMaterial;

class reEditableMap
{
private:
	std::string _fileName;
public:	
	int index;
	std::string fileName() const { return _fileName; }
	void fileName(std::string val) { _fileName = val; }

	QGLFramebufferObject *buffer, *textureBuffer;
	QImage mapImage;
	unsigned int unit;
	reVec2 textureSize;
	QPainter* currentPainter;
	
	reEditableMap();
	virtual void generate();
	virtual void bind();
	virtual void draw() {};
	void update();

	virtual QPainter* beginEditing();
	virtual void endEditing();	
};

class reTerrainMap: public reEditableMap
{
public:
	reShader normalShader;
	reTerrainMap();
	QGLFramebufferObject* normalFrameBuffer, *normalTextureBuffer;
	unsigned int normalTexture; 
	virtual void generate();
	virtual void draw();
	virtual void endEditing();
	virtual void bind();
	void updateNormalMap();
};

class reIndicatorMap: public reEditableMap
{
public:
	reVec2 position;
	virtual void draw();
};

class reSplatMap: public reEditableMap
{
public:
	virtual void draw();
};

class reTerrainRenderable: public reRenderable
{
public:
	reShader shader;
	int cols, rows;
	float size;
	std::string _mapDirectory;
	reTerrainMap textureMap;
	reSplatMap splatMap;
	reIndicatorMap brushMap;
	reTerrainMaterial* material;

	reTerrainRenderable();
	virtual void draw(int state = 0, int id = 0);
	void save();
	std::string mapDirectory() const { return _mapDirectory; }
	void mapDirectory(std::string val) { _mapDirectory = val; }
	void load();
};

class reTerrainNode: public reNode
{
public:
	reTerrainRenderable* terrainRenderable;
	reTerrainNode();
	virtual string className();
	reVec2 convertPoint( reVec3 point );

	virtual Json::Value toJson();
	virtual void loadJson(Json::Value& val);

};

#endif // reTerrainNode_h__