#ifndef reShader_h__
#define reShader_h__

#include <gl/glew.h>
#include <string>
#include <map>
#include "reTypes.h"

class reTexture;
class reRenderClient;

typedef std::map<std::string, int> reVarMap;
typedef std::map<std::string, int> reVarMap;

struct reShaderVar
{
	enum Kind 
	{
		uniform,
		sampler,
		attribute
	};
	Kind kind;
	std::string name;
	int id;
	int location;
	int index;
	int size;

	reShaderVar () { id = -1; };

	reShaderVar(const std::string& varName, int name, Kind kind = uniform, int size = 1)
	{
		this->name = varName;
		this->id = name;
		this->kind = kind;
		this->size = size;
	}
};

class reShader
{
private:
	std::string _vsFileName;
	std::string _fsFileName;

	GLuint program, vs, fs;
	reVarMap uniforms;	
	reVarMap textureUnits;

	reRenderClient* clients[20];

public:
	typedef enum
	{
		projMatrix,
		viewMatrix,		
		worldMatrix,
		diffuseColor,
		diffuseTexture,
		lightMatrices,
		boneMatrices,
		splatTextures,	
		shadowMap,
		lightNearDistance,
		lightFarDistance,
		farDistance,
		nearDistance,
		cellSize,

		position,
		normal,
		uv,
		boneWeights,
		boneIds,

		biasMatrix,
		lightMatrix,
		lightViewMatrix,
		lightProjMatrix
	} reUniformVar;

	std::vector<reShaderVar*> varTable;
	void registerVar(reShaderVar* uniform);

	reStringList defines;
	int effects;
	reShader();
	void use();

	void collectVars();

	void unuse();

	int location(int name, int index=0);
	void setUniform(int name, float v, int index = 0);
	void setUniform(int name, reVec2& v, int index = 0);
	void setUniform(int name, reVec4& v, int index = 0);
	void setUniform(int name, int v, int index = 0);
	void setUniform(int name, int count, int* v, int index = 0);
	void setUniformMatrix( int name, int count, GLfloat* matrices, int index = 0);
	void setAttribute(int name, reVec4 v, int index = 0);
	void setTexture(int name, reTexture* texture, int index = 0);
	void setTexture( int name, GLuint texture, int index = 0);
	
	std::string fsFileName() const;
	void fsFileName(std::string val);
	std::string vsFileName() const;
	void vsFileName(std::string val);
	std::string preprocess( std::string& src );
};

#endif // reShader_h__