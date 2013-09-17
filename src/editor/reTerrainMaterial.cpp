#include "reTerrainMaterial.h"

#include "json/json.h"
#include <gl/glew.h>

#include <fstream>

reTerrainLayer::reTerrainLayer()
{
	texture = 0;
}

reVar reTerrainLayer::toJson()
{
	reVar ret = reObject::toJson();
	ret["fileName"] = fileName();
	ret["tilingX"] = tiling().x;
	ret["tilingY"] = tiling().y;
	return ret;
}

void reTerrainLayer::loadJson( reVar& val )
{
	reObject::loadJson(val);
	fileName(val["fileName"].asString());
	tiling(reVec2(val["tilingX"].asFloat(), val["tilingY"].asFloat()));
}

std::string reTerrainLayer::fileName() const
{
	return _fileName;
}

void reTerrainLayer::fileName( std::string val )
{
	_fileName = val;
	delete texture;
	texture = new reTexture();
	texture->fileName(val);
}

float reTerrainLayer::rotation() const
{
	return _rotation;
}

void reTerrainLayer::rotation( float val )
{
	_rotation = val;
}

reVec2 reTerrainLayer::tiling() const
{
	return _tiling;
}

void reTerrainLayer::tiling( reVec2 val )
{
	_tiling = val;
}

void reTerrainLayer::bind( int idx )
{
	glActiveTexture(GL_TEXTURE0+idx);
	glEnable(GL_TEXTURE_2D);
	texture->bind(GL_TEXTURE0+idx);
}

//////////////////////////////////////////////////////////////////////////

reTerrainMaterial::reTerrainMaterial()
{
	add(layers = new reCollection());
}

void reTerrainMaterial::save( std::string afileName/*=""*/ )
{
	if (afileName.empty()) afileName = fileName();
	std::ofstream stream(afileName, std::ios::out);
	Json::StyledWriter writer;
	stream << writer.write(toJson());
	stream.close();
	fileName(afileName);
}

void reTerrainMaterial::load( std::string fileName )
{
	std::ifstream fs(fileName);
	Json::Reader reader;
	reVar val;
	reader.parse(fs, val);
	loadJson(val);
}

std::string reTerrainMaterial::fileName() const
{
	return _fileName;
}

void reTerrainMaterial::fileName( std::string val )
{
	_fileName = val;
}

void reTerrainMaterial::bind( int idx, reShader* shader )
{
	for (int i=0; i<layers->count(); i++)
	{
		((reTerrainLayer*)layers->at(i))->bind(idx+i);
	}
}

