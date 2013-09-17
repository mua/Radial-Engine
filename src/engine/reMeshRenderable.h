#include "reRenderable.h"
#include "reMesh.h"

class reMeshRenderable: public reRenderable
{
private:
	std::string _meshFileName;
	reMesh* _mesh;
	reBoneLinkList boneLinks;
	bool bonesMatched;
public:
	reMeshRenderable();
	~reMeshRenderable();
	void loadJson(reVar& value);
	void toJson(reVar& val, int categories = ALL_PROPERTIES);
	void draw(int state = 0 , int id = 0 );

	virtual void updateShader(reShader* shader) override;
	virtual void getTasks(reRenderTask& parent, reTaskList& tasks);
	virtual int getEffect() override;

	reMesh* mesh() const;
	void mesh(reMesh* val);
	std::string meshFileName() const;
	void meshFileName(std::string val);
	void matchBones();
	virtual string className();
};