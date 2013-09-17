#ifndef reImporter_h__
#define reImporter_h__

#include "std.h"
#include "reTypes.h"

class reNode;
class reMesh;
class reMaterial;
class reMaterialSet;
class reAnimator;
class reAnimset;

#define FBXSDK_NEW_API
#include <fbxsdk.h>

#include <vector>

struct reWeightData
{
	reWeightData():
		bones(-1, -1, -1, -1)
	{		
	}
	reVec4 weights;
	reVec4 bones;
};

typedef std::vector<reWeightData> reWeightList;
typedef std::vector<FbxNode*> reFbxNodeList;

class reFBXImpoter
{
private:
	QString _path;
	QString _dataDir;
	FbxScene* scene;
	FbxNode* sceneRoot;
	FbxImporter* importer;
public:
	reFBXImpoter();

	void import();

	void importFile();
	void importNode(FbxNode* fbxNode, reNode* parent, FbxScene* scene);
	reMesh* importMesh(FbxNode* fbxNode);
	reMaterial* getMaterial( FbxMesh* fmesh, int i, reMaterialSet& set);
	reVec2 getUV( FbxMesh* fmesh, int vi, int i, int j );
	reVec3 getNormal(FbxMesh* mesh, int vi);
	QString fileHash();

	QString path() const { return _path; }
	void path(QString val) { _path = val; }
	QString dataDir() const { return _dataDir; }
	void dataDir(QString val) { _dataDir = val; }
	void importAnimations(FbxNode* fnode, reAnimset* animset);
	void importNodeAnimations(reFbxNodeList& nodes, reNode* node);
};

#endif // reImporter_h__