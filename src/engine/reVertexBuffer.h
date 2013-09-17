#include "core.h"

struct reVertex;
class reShader;

class reVertexBuffer
{
public:
	GLuint boneLocation, weightLocation;
	GLuint vbo;
	reVertexBuffer();
	~reVertexBuffer();
	void load( reVertex* vertices, int count );
	void bindPointers(reShader* shader);
	void unbind();
};