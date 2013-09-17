#ifndef reFbxAsset_h__
#define reFbxAsset_h__

#include "reFileAsset.h"
#include "reTypes.h"
#include "reMesh.h"

#include <vector>
#include <QWidget>

class reNode;

using namespace std;

#define FBXSDK_NEW_API
#include <fbxsdk.h>


class reFBXAsset:
	public reFileAsset
{
public:
	reFBXAsset(reAsset* parent);
	virtual QWidget *createEditor(QWidget *parent);
	virtual void load();
	reFileAsset* meshes;

	void import();
	~reFBXAsset(void);
	reNode* importNode(FbxNode* fbxNode, reNode* parent);
	reMesh* importMesh(FbxNode* fbxNode);

	reMaterial* getMaterial( FbxMesh* fmesh, int i, reMaterialSet& set );
	reVec2 getUV( FbxMesh* fmesh, int vi, int i, int j );
	reVec3 getNormal(FbxMesh* mesh, int vi);

	QString fileHash();
	void loadNodeAssets();
	Json::Value getJson();
	void loadJson( Json::Value& val );
};
#endif // reFbxAsset_h__