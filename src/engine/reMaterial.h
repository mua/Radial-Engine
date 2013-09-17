#ifndef reMaterial_h__
#define reMaterial_h__

#include "reObject.h"
#include "reTypes.h"
#include "reRenderer.h"

#include <vector>
#include <string>

class reImage
{
public:
	int width, height;
	bool hasAlpha;
	char* data;
	~reImage();
};

class reTexture:
	public reObject
{
private:
	std::string _fileName;
	bool loaded;
public:	
	unsigned int glName;
	reTexture();
	~reTexture();
	std::string fileName() const;
	void fileName(std::string val);
	void bind(int unit);
	void load();
	void save(std::ostream& os);
	void load(std::istream& is);
	void loadFromFile(std::string fileName);
};

class reMaterial:
	public reObject, public reRenderClient
{
public:
	int id;
	reMaterial();
	~reMaterial();
	bool operator==(const reMaterial& other) const;
	bool isTransparent;

	reTexture* diffuseTexture;
	reColor4 diffuseColor;
	void bind();
	void unbind();
	void save(std::ostream& os);
	void load(std::istream& is);
	virtual void updateShader(reShader* shader) override;
	virtual int getEffect() override;
};

typedef std::vector<reMaterial*> reMaterialVector;

class reMaterialSet:
	public reObject
{
public:
	reMaterialVector materials;
	reMaterial* materialById(int id);
	void addMetarial(reMaterial* material);
	void save(std::ostream& os);
	void load(std::istream& is);
};

#endif // reMaterial_h__