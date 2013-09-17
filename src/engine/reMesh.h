#ifndef reMesh_h__
#define reMesh_h__

#include "reTypes.h"
#include "reMaterial.h"
#include "reRenderable.h"
#include "reShader.h"

#include <vector>
#include <string>

class reVertexBuffer;
class reNode;

struct reBoneLink
{
	char name[255];
	reMat4 linkMatrix;
	int id;
	reNode* bone;
};

struct reVertex
{
	reVec3 pos;
	reVec2 uv;
	reVec3 normal;
	reVec4 weights;
	reVec4 bones;
	reVertex(const reVec3& apos):
		pos(apos)
	{
	}
	reVertex() {}
};

typedef std::vector<reVertex> reVertexVector;

struct reFace 
{
	reVertex vertices[3];
	void save(std::ostream& os);
	void load(std::istream& is);
};

typedef std::vector<reFace> reFaceVector;
typedef std::vector<reBoneLink> reBoneLinkList;

struct reGroup: public reRenderable
{
	reVertexBuffer* vertexBuffer;
	reGroup();
	~reGroup();
	reFaceVector faces;
	reMaterial* material;
	bool bufferChanged;
	void save(std::ostream& os);
	void reGroup::load( std::istream& is, reMaterialSet& materials );

	void updateBuffer();

	virtual void draw(int state = 0, int id = 0);

	virtual void updateShader(reShader* shader) override;
	virtual void getTasks(reRenderTask& parent, reTaskList& tasks);
};

typedef std::vector<reGroup*> reGroupVector;

class reMesh: public reObject
{
private:
	std::string _path;
	reBBox _bBox;	
protected:
	bool loaded;
public:	
	reShader shader;
	bool hasSkin;
	reBoneLinkList boneLinks;

	reGroupVector groups;
	reMaterialSet materialSet;

	reMesh();
	~reMesh();
	
	reGroup* groupByMaterial(reMaterial* material);
	void addFace(reFace& face, int matId);

	void save(std::ostream& os);
	void load(std::istream& is);
	virtual void load();

	void save(std::string& fileName);
	void load(std::string& fileName);
	virtual void loadFromFile(string fileName);

	std::string path() const;
	void path(std::string val);

	reBBox bBox();
	void bBox(reBBox val);

	virtual string className();
};

#endif // reMesh_h__
