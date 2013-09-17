#ifndef reMeshAsset_h__
#define reMeshAsset_h__

#include "reFileAsset.h"
#include "reMesh.h"

#include <QVariant>

class reMeshAsset:
	public reFileAsset
{
public:
	reMesh* mesh;
	reMeshAsset(reFileAsset* parent);
};

#endif // reMeshAsset_h__