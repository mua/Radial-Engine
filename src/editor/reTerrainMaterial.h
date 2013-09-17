#ifndef reTerrainMaterial_h__
#define reTerrainMaterial_h__

#include "reTypes.h"
#include "reMaterial.h"
#include "reShader.h"

#include <json/json.h>

#include <string>
#include <vector>

#include "reObject.h"
#include "reCollection.h"

class reTerrainLayer: public reObject
{
private:
	std::string _fileName;
	reVec2 _tiling;
	float _rotation;
	reTexture* texture;
public:	
	reTerrainLayer();
	virtual Json::Value toJson();
	virtual void loadJson(Json::Value& value);

	std::string fileName() const;
	void fileName(std::string val);	
	float rotation() const;
	void rotation(float val);
	reVec2 tiling() const;
	void tiling(reVec2 val);

	void bind(int idx);
};
typedef std::vector<reTerrainLayer*> reTerrainLayerVector;

class reTerrainMaterial: public reCollection
{
private:
	std::string _fileName;
public:
	reCollection* layers;

	reTerrainMaterial();
	void save(std::string fileName="");
	void load(std::string fileName);

	std::string fileName() const;
	void fileName(std::string val);	

	void bind( int idx, reShader* shader );
};

#endif // reTerrainMaterial_h__
