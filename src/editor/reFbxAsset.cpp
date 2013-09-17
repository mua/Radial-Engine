#include "reFBXAsset.h"

#include "reMeshAsset.h"
#include "reNodeAsset.h"
#include "reNode.h"

#include <QBoxLayout>
#include <QtCore/QDate>
#include <QtCore/QLocale>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QDebug>
#include <QDir>
#include <QDirIterator>

#include "utils.h"

#include <json/json.h>
#include <istream>
#include <fstream>

FbxManager* sdk = 0;

reFBXAsset::reFBXAsset(reAsset* parent):
	reFileAsset(parent)
{
	editor = NULL;
	if (!sdk)
	{
		FbxIOSettings *ios = FbxIOSettings::Create(sdk, IOSROOT);
		sdk = FbxManager::Create();
		sdk->SetIOSettings(ios);
	}
}

reFBXAsset::~reFBXAsset(void)
{
	delete editor;
}

QWidget* reFBXAsset::createEditor( QWidget *parent )
{
	editor = new QWidget(parent);
	return editor;
}
/*
void reFBXAsset::importMaterials(FbxMesh* pMesh)
{
	int i, l, lPolygonCount = pMesh->GetPolygonCount();

	for (i = 0; i < lPolygonCount; i++)
	{
		for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
		{

			FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
			FbxSurfaceMaterial* lMaterial = NULL;
			int lMatId = -1;
			lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(i));
			lMatId = lMaterialElement->GetIndexArray().GetAt(i);

			if(lMatId >= 0)
			{
				DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
			}
		}
	}
};
*/

void reFBXAsset::loadNodeAssets () 
{
	QDirIterator it(dataDir(), QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
	while (it.hasNext())
	{
		QFileInfo info(it.next());
		if (info.completeSuffix() == "json") 
		{
			reFileAsset* asset = new reFileAsset(this);
			asset->setPath(info.filePath());
			//reNodeAsset* nodeAsset = new reNodeAsset(this);
			//nodeAsset->nodeFilePath(info.filePath().toStdString());
			//nodeAsset->node(node, true);
			children.push_back(asset);
		}
		if (info.completeSuffix() == "mesh") 
		{
			reFileAsset* asset = new reMeshAsset(this);
			asset->setPath(info.filePath());
			children.push_back(asset);
		}

	}
}

void reFBXAsset::load()
{
	clearChildren();
	bool importNeeded = true;
	ifstream ifs(metaPath().toStdString().c_str(), ios::in);
	if (!ifs.fail())
	{
		Json::Reader reader;
		Json::Value root;
		reader.parse(ifs, root);
		QString hash = root["hash"].asString().c_str();
		if (fileHash() == hash) 
		{
			loadJson(root);
			importNeeded = false;
		}
	}

	if (importNeeded)
		import();

	loadNodeAssets();

	Json::StyledWriter writer;
	ofstream fs(metaPath().toStdString().c_str(), ios::out);
	fs << writer.write(getJson());
	fs.close();
}

Json::Value reFBXAsset::getJson()
{
	Json::Value val;
	val["children"] = Json::Value(Json::arrayValue);
	val["name"] = path().toStdString();
	val["hash"] = fileHash().toStdString();	
	return val;
}

void reFBXAsset::loadJson( Json::Value& val )
{
	for (unsigned int i=0; i<val["children"].size(); i++)
	{
		Json::Value childValue = val["children"][i];
		reAsset* child = reAsset::createChild(childValue["class"].asString().c_str(), this);
		child->loadJson(childValue);
		children.push_back(child);

	}
}

QString reFBXAsset::fileHash()
{
	int hash = int(info.lastModified().toMSecsSinceEpoch()/1000) + info.size();
	qDebug() << QString(hash);
	QString ret; 
	ret.setNum(hash);
	return ret;
}

void reFBXAsset::import()
{
	FbxImporter* importer = FbxImporter::Create(sdk,"");
	meshes = new reFileAsset(this);
	meshes->setPath("meshes");
	children.push_back(meshes);
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
	if(lRootNode) {
		reNode* node = importNode(lRootNode, NULL);
		node->name(info.baseName().toStdString());
		QString path = dataDir() + QDir::separator() + node->name().c_str() + ".json";
		node->save(path.toStdString());
	}
	importer->Destroy();
}

reVec3 reFBXAsset::getNormal(FbxMesh* fmesh, int vi)
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

reMesh* reFBXAsset::importMesh(FbxNode* fbxNode)
{
	qDebug() << "import mesh for" << fbxNode->GetName();
	reMesh* mesh = new reMesh;

	FbxMesh* fmesh = (FbxMesh*) fbxNode->GetNodeAttribute();
	FbxVector4* controlPoints = fmesh->GetControlPoints(); 

	for (int i=0; i<fmesh->GetPolygonCount(); i++) 
	{
		reFace* face = new reFace;
		for (int j=0; j<fmesh->GetPolygonSize(i); j++)
		{
			int vi = fmesh->GetPolygonVertex(i, j);
			reVertex vertex;
			vertex.pos = reVec3(controlPoints[vi][0], controlPoints[vi][1], controlPoints[vi][2]);
			FbxVector4 fNormal;
			fmesh->GetPolygonVertexNormal(i, j, fNormal);
			vertex.uv = getUV(fmesh, vi, i, j);
			vertex.normal = reVec3(fNormal[0], fNormal[1], fNormal[2]);
			face->vertices.push_back(vertex);	
		}
		reMaterial* mat =  getMaterial(fmesh, i, mesh->materialSet);
		mesh->addFace(face,mat ? mat->id: -1);
	}
	
	reMeshAsset* meshAsset = new reMeshAsset(meshes);
	meshAsset->mesh = mesh;
	meshes->children.push_back(meshAsset);
	meshAsset->setPath((dataDir().toStdString() + "/" + fbxNode->GetName() + ".mesh").c_str());
	mesh->save(dataDir().toStdString() + "/" + fbxNode->GetName() + ".mesh");
	return mesh;
}

glm::mat4 fromFBXMatrix(FbxMatrix& fm)
{
	glm::mat4 mat;
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			mat[i][j] = fm.Get(i, j);
		}
	}
	return mat;
}

reNode* reFBXAsset::importNode(FbxNode* fbxNode, reNode* parent)
{
	reNode* node = new reNode;
	node->name(fbxNode->GetName());
	FbxMatrix mat = fbxNode->EvaluateGlobalTransform(FbxTime(0));

	FbxNode *fbxParent = fbxNode->GetParent();
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
			reMesh* mesh = importMesh(fbxNode);
			node->renderables->add(mesh);			
			break;	
		}
	}

	for(int i = 0; i < fbxNode->GetChildCount(); i++) 
	{
		reNode* child = importNode(fbxNode->GetChild(i), node);
		node->children->add(child);
	}
	return node;
};

reVec2 reFBXAsset::getUV( FbxMesh* fmesh, int vi, int i, int j )
{
	for (int l = 0; l < fmesh->GetElementUVCount(); ++l)
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
}

reMaterial* reFBXAsset::getMaterial( FbxMesh* fmesh, int i, reMaterialSet& set)
{
	reMaterial* mat = NULL;
	for (int l = 0; l < fmesh->GetElementMaterialCount(); l++)
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
			if (lNbTextures)
			{
				mat->diffuseTexture = new reTexture;
				FbxTexture* lTexture = FbxCast <FbxTexture> (lProperty.GetSrcObject(FbxTexture::ClassId,0));
				qDebug() << "map: " << lTexture->GetName();
				FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(lTexture);
				if (lFileTexture)
				{
					mat->diffuseTexture->fileName(lFileTexture->GetFileName());
				}

			}
			//////////////////////////////////////////////////////////////////////////
		}
	}
	return mat;
}