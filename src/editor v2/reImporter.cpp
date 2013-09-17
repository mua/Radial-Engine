#include "std.h"
#include "reImporter.h"

#include "reNode.h"
#include "reMesh.h"
#include "reMaterial.h"
#include "reAnimationSet.h"
#include "reBone.h"
#include "rePhysics.h"
#include "reBody.h"
#include "reMeshRenderable.h"

#include <json/json.h>
#include <vector>
#include <fstream>

using namespace std;

FbxManager* sdk = 0;
glm::mat4 fromFBXMatrix(FbxMatrix& fm);

reFBXImpoter::reFBXImpoter()
{
	if (!sdk)
	{
		FbxIOSettings *ios = FbxIOSettings::Create(sdk, IOSROOT);
		sdk = FbxManager::Create();
		sdk->SetIOSettings(ios);
	}
}

QString reFBXImpoter::fileHash()
{
	QFileInfo info(path());
	int hash = int(info.lastModified().toMSecsSinceEpoch()/1000) + info.size();
	qDebug() << QString(hash);
	QString ret; 
	ret.setNum(hash);
	return ret;
}

void reFBXImpoter::import()
{
	QString hash = fileHash();
	QString infoFile(QDir(dataDir()).absoluteFilePath("import.json"));
	{		
		ifstream is(infoFile.toStdString(), ios::in);
		if (!is.fail())
		{
			Json::Reader reader;
			Json::Value val;
			reader.parse(is, val);
			if (val["hash"].asString() == hash.toStdString() && !path().contains("walker"))
			{
				qDebug() << "import is skipped, file is not changed.";
				return;
			}
		}
	}
	importFile();
	qDebug() << "import complete! : " << path();

	qDebug() << "saving! : " << infoFile;
	Json::Value val;
	val["hash"] = fileHash().toStdString();
	ofstream os(infoFile.toStdString(), ios::out);
	os << val;
}

reVec3 reFBXImpoter::getNormal(FbxMesh* fmesh, int vi)
{
	FbxVector4 normal;
	for(int l = 0; l < fmesh->GetElementNormalCount(); ++l)
	{
		FbxGeometryElementNormal* leNormal = fmesh->GetElementNormal(l);
		if(leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			switch (leNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				normal = leNormal->GetDirectArray().GetAt(vi);
				return reVec3(normal[0], normal[1], normal[2]);
			case FbxGeometryElement::eIndexToDirect:
				int id = leNormal->GetIndexArray().GetAt(vi);
				normal = leNormal->GetDirectArray().GetAt(id);
				return reVec3(normal[0], normal[1], normal[2]);
			}
		}
	}
	return reVec3(0,0,0);
}

bool boneSort (reBoneLink a,reBoneLink b) 
{ 
	return (a.id<b.id); 
}

bool getSkinData(FbxMesh* mesh, reBoneLinkList& bones, reWeightList& weights)
{
	qDebug() << "------------------------------------";
	int lSkinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);	

	if (!lSkinCount)
	{
		return false;
	}

	assert(lSkinCount == 1);

	int lVertexCount = mesh->GetControlPointsCount();
	weights.resize(lVertexCount);

	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)mesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{			
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			FbxCluster::ELinkMode lClusterMode = lCluster->GetLinkMode();
			assert(lClusterMode == FbxCluster::eNormalize);
			if (!lCluster->GetLink())
				continue;
			qDebug() << lCluster->GetLink()->GetName();

			reBoneLink link;
			FbxAMatrix linkMatrix, refMatrix, tmp;

			FbxNode* pNode = mesh->GetNode();
			const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
			const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
			const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

			lCluster->GetTransformMatrix(refMatrix);
			refMatrix *= FbxAMatrix(lT, lR, lS);
			lCluster->GetTransformLinkMatrix(linkMatrix);

			link.linkMatrix = fromFBXMatrix(FbxMatrix(linkMatrix.Inverse()) * refMatrix);

			strcpy_s(link.name, sizeof(link.name), lCluster->GetLink()->GetName());
			link.id = lCluster->GetLink()->GetUniqueID();
			bones.push_back(link);
		}

		reBoneLinkList sortedBones = bones;
		sort(sortedBones.begin(), sortedBones.end(), boneSort);

		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{	
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for ( int k = 0; k < lVertexIndexCount; ++k) 
			{            
				int lIndex = lCluster->GetControlPointIndices()[k];
				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
				{
					continue;
				}

				int boneSlot;
				for (boneSlot = 0; boneSlot<4; boneSlot++)
					if (weights[lIndex].bones[boneSlot] == -1)
						break;
				if (boneSlot > 3)
				{
					continue;
				}
				assert(weights[lIndex].bones[boneSlot] == -1);
				size_t boneIndex;
				for (boneIndex = 0; boneIndex<sortedBones.size(); boneIndex++)
				{
					if (bones[lClusterIndex].id == sortedBones[boneIndex].id) break;
				}
				weights[lIndex].bones[boneSlot] = boneIndex;
				weights[lIndex].weights[boneSlot] = lWeight;
				qDebug() << lWeight;
			}		
		}
		bones = sortedBones;
	}
	
	for ( int i=0; i<weights.size(); i++)
	{
		float sum = 0;
		for ( int k=0; k<4; k++)
		{
			sum += weights[i].weights[k];
		}
		//assert(abs(sum - 1) < 0.0001f);
	}
	return true;
};

reMesh* reFBXImpoter::importMesh(FbxNode* fbxNode)
{
	qDebug() << "import mesh for" << fbxNode->GetName();
	reMesh* mesh = new reMesh;

	FbxMesh* fmesh = (FbxMesh*) fbxNode->GetNodeAttribute();
	FbxVector4* controlPoints = fmesh->GetControlPoints(); 	

	reWeightList weights;
	reBoneLinkList boneLinks;

	if (mesh->hasSkin = getSkinData(fmesh, boneLinks, weights))
	{
		qDebug() << "imported skin data";
	}	

	for (int i =0; i<fmesh->GetPolygonCount(); i++) 
	{
		reFace face;
		for ( int j=0; j<fmesh->GetPolygonSize(i); j++)
		{
			int vi = fmesh->GetPolygonVertex(i, j);
			reVertex vertex;
			vertex.pos = reVec3(controlPoints[vi][0], controlPoints[vi][1], controlPoints[vi][2]);
			FbxVector4 fNormal;
			fmesh->GetPolygonVertexNormal(i, j, fNormal);
			vertex.uv = getUV(fmesh, vi, i, j);
			vertex.normal = reVec3(fNormal[0], fNormal[1], fNormal[2]);
			if (mesh->hasSkin)
			{
				vertex.bones = weights[vi].bones;
				vertex.weights = weights[vi].weights;
			}
			assert(j<3);
			face.vertices[j] = vertex;
		}
		reMaterial* mat = getMaterial(fmesh, i, mesh->materialSet);
		mesh->addFace(face, mat ? mat->id: -1);
	}
	if (mesh->hasSkin)
		mesh->boneLinks = boneLinks;
	mesh->save(dataDir().toStdString() + "/" + fbxNode->GetName() + ".mesh");
	if (mesh->materialSet.materials[0]->diffuseTexture)
		qDebug() << "found";

	return mesh;
}

FbxNode* getFbxParent(FbxNode* fbxNode)
{
	FbxNode* parent = fbxNode->GetParent();
	return parent;

	if (parent && parent->GetNodeAttribute() && 
		parent->GetNodeAttribute()->GetAttributeType() && 
		parent->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		FbxSkeleton* lSkeleton = (FbxSkeleton*) parent->GetNodeAttribute();
		if (lSkeleton->GetSkeletonType() != FbxSkeleton::eLimbNode)
		{
			return getFbxParent(parent);
		}
	}
	return parent;
};

void reFBXImpoter::importNode(FbxNode* fbxNode, reNode* parent, FbxScene* scene)
{
	reNode* node = 0;
	if (fbxNode->GetNodeAttribute() )
	{
		FbxNodeAttribute::EType attrType = (fbxNode->GetNodeAttribute()->GetAttributeType());	
		switch (attrType)
		{
		case FbxNodeAttribute::eSkeleton: 
			//if (((FbxSkeleton*)fbxNode->GetNodeAttribute())->GetSkeletonType() != FbxSkeleton::eLimbNode) 
				//return;
			node = new reBone;
			break;
		}
	}
	if (!node)
	{
		node = new reNode;
	}
	node->name(fbxNode->GetName());
	qDebug() << fbxNode->GetName() << ":" << fbxNode->GetUniqueID();
	fbxNode->SetUserDataPtr(node);
	//FbxMatrix mat = fbxNode->EvaluateGlobalTransform(FbxTime(0));

	FbxVector4 t = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 s = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxVector4 r = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxMatrix offset (t,r,s);
	FbxMatrix globalTransform = fbxNode->EvaluateGlobalTransform();
	FbxMatrix mat = globalTransform * offset;

	FbxNode *fbxParent = getFbxParent(fbxNode);
	if (!fbxParent)
	{
		node->transform (reTransform(fromFBXMatrix(mat)));
	}	
	else
	{
		FbxMatrix pmat = fbxParent->EvaluateGlobalTransform(FbxTime(0));
		node->transform(reTransform(fromFBXMatrix(pmat.Inverse()) * fromFBXMatrix(mat)));
	}

	if (fbxNode->GetNodeAttribute() )
	{
		FbxNodeAttribute::EType attrType = (fbxNode->GetNodeAttribute()->GetAttributeType());	
		switch (attrType)
		{
		case FbxNodeAttribute::eMesh:      
			//reBody* body = new reBody;
			//node->add(body);
			//body->node(node);
			reMesh* mesh = importMesh(fbxNode);
			reMeshRenderable* meshRenderable = new reMeshRenderable;
			meshRenderable->mesh(mesh); 
			node->renderables->add(meshRenderable);
			break;
		}
	}	

	for(int i = 0; i < fbxNode->GetChildCount(); i++) 
	{
		importNode(fbxNode->GetChild(i), node, scene);		
	}
	if (fbxParent)
	{
		((reNode*)fbxParent->GetUserDataPtr())->children->add(node);
	}

};

void reFBXImpoter::importAnimations(FbxNode* fnode, reAnimset* animset)
{
	FbxAnimStack* lAnimStack = scene->GetSrcObject<FbxAnimStack>(0);
	if (!lAnimStack)
	{
		return;
	}
	double duration = lAnimStack->GetLocalTimeSpan().GetDuration().GetSecondDouble();
	float fps = 30;
	int frames = int(duration*30);
	FbxTime time;
	time.SetGlobalTimeMode(FbxTime::eFrames30);

	FbxNode *fbxParent = getFbxParent(fnode);

	for ( int i=0; i<frames; i++)
	{
		time.SetFrame(i);

		reMat4 pmat;
		if (fbxParent)
		{
			FbxMatrix pfmat = fbxParent->EvaluateGlobalTransform(time).Inverse();
			pmat = fromFBXMatrix(pfmat);
		}		
		FbxMatrix mat = fnode->EvaluateGlobalTransform(time);
		//qDebug() << reVar(reTransform(pmat * fromFBXMatrix(mat))).asJson().toStyledString().c_str();
		reTransformPose* pose = new reTransformPose;
		pose->nodeName(fnode->GetName());
		pose->transform(reTransform(pmat * fromFBXMatrix(mat)));
		animset->addPose(i, (reNode*)fnode->GetUserDataPtr(), pose);
	}
};

reVec2 reFBXImpoter::getUV( FbxMesh* fmesh, int vi, int i, int j )
{
	for ( int l = 0; l < fmesh->GetElementUVCount(); ++l)
	{
		FbxGeometryElementUV* leUV = fmesh->GetElementUV(l);

		switch (leUV->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (leUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					FbxVector2 uv = leUV->GetDirectArray().GetAt(vi);
					return reVec2(uv[0], uv[1]);
					break;
				}

			case FbxGeometryElement::eIndexToDirect:
				{
					int id = leUV->GetIndexArray().GetAt(vi);
					FbxVector2 uv = leUV->GetDirectArray().GetAt(id);
					return reVec2(uv[0], uv[1]);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			{
				int lTextureUVIndex = fmesh->GetTextureUVIndex(i, j);
				switch (leUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				case FbxGeometryElement::eIndexToDirect:
					{
						FbxVector2 uv = leUV->GetDirectArray().GetAt(lTextureUVIndex);
						return reVec2(uv[0], uv[1]);
					}
					break;
				default:
					break; // other reference modes not shown here!
				}
			}
			break;
		case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
		case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
		case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
			break;
		}
	}
	return reVec2(0,0);
}

reMaterial* reFBXImpoter::getMaterial( FbxMesh* fmesh, int i, reMaterialSet& set)
{
	reMaterial* mat = NULL;
	for ( int l = 0; l < fmesh->GetElementMaterialCount(); l++)
	{		
		FbxGeometryElementMaterial* lMaterialElement = fmesh->GetElementMaterial(l);
		int lMatId = lMaterialElement->GetIndexArray().GetAt(i);
		if(lMatId >= 0)
		{ 
			if (mat = set.materialById(lMatId))
				return mat;
			mat = new reMaterial;
			mat->id = lMatId;
			set.addMetarial(mat);
			FbxSurfaceMaterial* lMaterial = fmesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(i));
			if (!lMaterial)
			{
				continue; 
			}
			//qDebug() << lMaterial->GetDoubleParameter(FbxSurfaceMaterial::sTransparencyFactor);
			//////////////////////////////////////////////////////////////////////////
			FbxProperty lProperty = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if (lMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				FbxDouble3 lFbxDouble3;
				lFbxDouble3 =((FbxSurfacePhong *)lMaterial)->Diffuse;
				mat->diffuseColor = reColor4(lFbxDouble3[0], lFbxDouble3[1], lFbxDouble3[2], 1);
			}
			if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				FbxDouble3 lFbxDouble3;
				lFbxDouble3 =((FbxSurfaceLambert *)lMaterial)->Diffuse;
				mat->diffuseColor = reColor4(lFbxDouble3[0], lFbxDouble3[1], lFbxDouble3[2], 1);
			}

			////////////////////////////////////////////////////////////////////////// read texture
			int lNbTextures = lProperty.GetSrcObjectCount(FbxTexture::ClassId);
			FbxTexture* lTexture = 0;
			FbxFileTexture *lFileTexture;
			if (lNbTextures)
			{
				mat->diffuseTexture = new reTexture;
				lTexture = FbxCast <FbxTexture> (lProperty.GetSrcObject(FbxTexture::ClassId,0));
				qDebug() << lTexture->GetAlphaSource();
				qDebug() << "map: " << lTexture->GetName();
				lFileTexture = FbxCast<FbxFileTexture>(lTexture);
				if (lFileTexture)
				{
					qDebug() << lFileTexture->Alpha;
					mat->diffuseTexture->fileName(lFileTexture->GetFileName());
				}

			}
			//////////////////////////////////////////////////////////////////////////
			lProperty = lMaterial->FindProperty(FbxSurfaceMaterial::sTransparentColor);
			if (lProperty.GetSrcObjectCount(FbxTexture::ClassId))
			{
				mat->isTransparent = (strcmpi(lFileTexture->GetFileName(), FbxCast<FbxFileTexture>(lProperty.GetSrcObject<FbxTexture>(0))->GetFileName()) == 0);
			}
		}
	}
	if (!mat)
	{
		if (set.materials.size())
		{
			return set.materials[0];
		}
		mat = new reMaterial;
		mat->id = 0;
		FbxDouble3 lFbxDouble3 = fmesh->Color;	
		mat->diffuseColor = reColor4(lFbxDouble3[0], lFbxDouble3[1], lFbxDouble3[2], 1);
		set.addMetarial(mat);
	}
	return mat;
}

void reFBXImpoter::importNodeAnimations(reFbxNodeList& nodes, reNode* node)
{
	reAnimset *animset = new reAnimset();
	for (size_t i=0; i<nodes.size(); i++)
	{
		importAnimations(nodes[i], animset);
	}	
	QString path = dataDir() + QDir::separator() + node->name().c_str() + ".json";
	QString animpath = dataDir() + QDir::separator() + QFileInfo(this->path()).baseName() + "_animations.json";
	animset->saveToFile(animpath.toStdString());
	reAnimator* animator = new reAnimator;
	animator->animset(animset);
	node->add(animator);
}

void reFBXImpoter::importFile()
{
	importer = FbxImporter::Create(sdk,"");
	if(!importer->Initialize(path().toUtf8().constData(), -1, sdk->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", importer->GetLastErrorString());
		return;
	}
	FbxScene* lScene = FbxScene::Create(sdk, "myScene");
	FbxAxisSystem ga(FbxAxisSystem::OpenGL);
	ga.ConvertScene(lScene);
	importer->Import(lScene);	
	FbxNode* lRootNode = lScene->GetRootNode();
	scene = lScene; 
	sceneRoot = lRootNode;
	if(lRootNode) 
	{
		importNode(lRootNode, NULL, lScene);		
		reNode* node = (reNode*)lRootNode->GetUserDataPtr();
		reNode* skeleton = node->findObject<reBone>();
		if (skeleton)
		{
			while (dynamic_cast<reBone*>(skeleton)) 
				skeleton = skeleton->parent();
			reMeshRenderable* meshRenderable = node->findObject<reMeshRenderable>();			
			if (meshRenderable->mesh() && meshRenderable->mesh()->hasSkin)
			{
				reMesh* mesh = meshRenderable->mesh();
				reFbxNodeList list;
				for (int i=0; i<mesh->boneLinks.size(); i++)
				{
					list.push_back(scene->FindNodeByName(mesh->boneLinks[i].name));
				}
				importNodeAnimations(list, skeleton);
			}
		}
		else
		{
			reFbxNodeList list;
			for (int i=0; i<scene->GetNodeCount(); i++)
				list.push_back(scene->GetNode(i));
			importNodeAnimations(list, node);
		}		
		node->name(QFileInfo(path()).baseName().toStdString());
		QString path = dataDir() + QDir::separator() + node->name().c_str() + ".json";
		node->save(path.toStdString());		
	}

	/*
	for(int lAnimStackCount=0; lAnimStackCount < importer->GetAnimStackCount(); lAnimStackCount++) 
	{
		FbxTakeInfo* lTakeInfo = importer->GetTakeInfo(lAnimStackCount);
		FbxString lTakeName = lTakeInfo->mName;	
		qDebug() << lTakeName.Buffer() << lTakeInfo->mReferenceTimeSpan.GetStart().GetSecondDouble();
	}
	*/
	delete ((reNode*)lRootNode->GetUserDataPtr());
	lScene->Destroy();
	importer->Destroy();	
}

//////////////////////////////////////////////////////////////////////////

glm::mat4 fromFBXMatrix(FbxMatrix& fm)
{
	glm::mat4 mat;
	for (unsigned int i =0; i<4; i++)
	{
		for ( int j=0; j<4; j++)
		{
			mat[i][j] = fm.Get(i, j);
		}
	}
	return mat;
}
